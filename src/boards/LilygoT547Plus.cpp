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

#ifdef DUAL_BUTTON_MODE
  #include "controls/DualButtonControls.h"
  #ifndef BUTTON_FWD_GPIO
    #define BUTTON_FWD_GPIO GPIO_NUM_45
  #endif
  #ifndef BUTTON_BWD_GPIO
    #define BUTTON_BWD_GPIO GPIO_NUM_48
  #endif
#else
  #include "controls/SingleButtonControls.h"
  #ifndef BUTTON_SELECT_GPIO
    #define BUTTON_SELECT_GPIO GPIO_NUM_21
  #endif
#endif

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
#ifdef DUAL_BUTTON_MODE
  return new DualButtonControls(
      BUTTON_FWD_GPIO, BUTTON_BWD_GPIO,
      [ui_queue](UIAction action) { xQueueSend(ui_queue, &action, 0); });
#else
  return new SingleButtonControls(
      BUTTON_SELECT_GPIO,
      [ui_queue](UIAction action) { xQueueSend(ui_queue, &action, 0); });
#endif
}
