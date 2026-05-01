#pragma once

#include "ButtonControls.h"
#include "Actions.h"
#include "GPIOButton.h"
#include <hal/gpio_types.h>

// Single-button controls for the Lilygo T5-4.7-Plus (ESP32-S3).
// The board has only one button so it always maps to SELECT.
// UP/DOWN are handled via the touch controls layer instead.
// Deep sleep wakeup is handled via EXT0 on GPIO 21.
class SingleButtonControls : public ButtonControls
{
private:
  gpio_num_t m_gpio;
  ActionCallback_t m_on_action;
  GPIOButton *m_button = nullptr;

public:
  SingleButtonControls(gpio_num_t gpio, ActionCallback_t on_action);

  bool did_wake_from_deep_sleep() override;
  UIAction get_deep_sleep_action() override;
  void setup_deep_sleep() override;
};
