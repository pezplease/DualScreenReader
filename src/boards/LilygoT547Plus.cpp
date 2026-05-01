#include "LilygoT547Plus.h"
// epdiy.h defines EpdFont/EpdGlyph — must precede the font headers
#include <epdiy.h>
#include <regular_font.h>
#include <bold_font.h>
#include <italic_font.h>
#include <bold_italic_font.h>
#include <hourglass.h>
#include <epd_driver.h>    // lilygo driver: epd_lily_init/poweron/poweroff
#include <Renderer/LilygoT547Renderer.h>
#include "controls/SingleButtonControls.h"

// The T5-4.7-Plus (ESP32-S3) has a single button on GPIO 21 (active low)
#define BUTTON_SELECT_GPIO GPIO_NUM_21

void LilygoT547Plus::power_up()
{
  // Initialise the lilygo display driver and power on the panel.
  // This must happen before the renderer allocates its framebuffer.
  epd_lily_init();
  epd_lily_poweron();
}

void LilygoT547Plus::prepare_to_sleep()
{
  epd_lily_poweroff();
}

Renderer *LilygoT547Plus::get_renderer()
{
  return new LilygoT547Renderer(
      &regular_font,
      &bold_font,
      &italic_font,
      &bold_italic_font,
      hourglass_data,
      hourglass_width,
      hourglass_height);
}

TouchControls *LilygoT547Plus::get_touch_controls(Renderer *renderer, xQueueHandle ui_queue)
{
  // Return dummy touch controls — touch via L58 can be added later
  return new TouchControls();
}

ButtonControls *LilygoT547Plus::get_button_controls(xQueueHandle ui_queue)
{
  return new SingleButtonControls(
      BUTTON_SELECT_GPIO,
      [ui_queue](UIAction action)
      {
        xQueueSend(ui_queue, &action, 0);
      });
}
