#pragma once

#include <esp_heap_caps.h>  // heap_caps_malloc for PSRAM allocation
#include <epd_driver.h>     // lilygo driver — epd_lily_* hardware functions
#include <epdiy.h>          // epdiy drawing functions and rotation
#include "EpdiyFrameBufferRenderer.h"

// Physical display dimensions for the Lilygo T5-4.7 (landscape orientation).
// Drawing functions address [0..EPD_WIDTH-1 x 0..EPD_HEIGHT-1].
// In inverted-portrait mode the readable page is EPD_HEIGHT wide x EPD_WIDTH tall.
#define EPD_WIDTH  960
#define EPD_HEIGHT 540

// Renderer for the Lilygo T5-4.7-Plus (ESP32-S3).
//
// Hardware path: lilygo epd_driver (epd_lily_*) — this is the only path that
// works on ESP32-S3. The upstream epdiy board definitions use I2S which is an
// ESP32-only peripheral.
//
// Software path: epdiy drawing functions (epd_draw_pixel, epd_fill_rect, …)
// — these are pure software operations on the framebuffer and work on any
// target without calling epd_init().
class LilygoT547Renderer : public EpdiyFrameBufferRenderer
{
public:
  LilygoT547Renderer(
      const EpdFont *regular_font,
      const EpdFont *bold_font,
      const EpdFont *italic_font,
      const EpdFont *bold_italic_font,
      const uint8_t *busy_icon,
      int busy_icon_width,
      int busy_icon_height)
      : EpdiyFrameBufferRenderer(regular_font, bold_font, italic_font, bold_italic_font, busy_icon, busy_icon_width, busy_icon_height)
  {
    // Display is already powered on by LilygoT547Plus::power_up().
    // Just set the rotation and allocate a PSRAM framebuffer.
    epd_set_rotation(EPD_ROT_INVERTED_PORTRAIT);
    m_frame_buffer = (uint8_t *)heap_caps_malloc(EPD_WIDTH * EPD_HEIGHT / 2, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    memset(m_frame_buffer, 0xFF, EPD_WIDTH * EPD_HEIGHT / 2); // all white
  }

  ~LilygoT547Renderer()
  {
    free(m_frame_buffer);
    m_frame_buffer = nullptr;
  }

  void flush_display() override
  {
    Rect_t area = epd_lily_full_screen();
    epd_lily_clear_area(area);
    epd_lily_draw_grayscale_image(area, m_frame_buffer);
    needs_gray_flush = false;
  }

  // Partial-area flush: no pre-clear needed for small/transient updates (e.g. busy spinner)
  void flush_area(int x, int y, int width, int height) override
  {
    Rect_t area = {.x = x, .y = y, .width = width, .height = height};
    epd_lily_draw_grayscale_image(area, m_frame_buffer);
  }

  void reset() override
  {
    // Hardware full-clear (flashes white), then blank the software buffer
    epd_lily_clear();
    memset(m_frame_buffer, 0xFF, EPD_WIDTH * EPD_HEIGHT / 2);
  }

  void clear_screen() override
  {
    // Software-only clear — no hardware flash
    memset(m_frame_buffer, 0xFF, EPD_WIDTH * EPD_HEIGHT / 2);
  }
};
