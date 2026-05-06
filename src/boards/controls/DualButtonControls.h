#pragma once

#include "ButtonControls.h"
#include "Actions.h"
#include <hal/gpio_types.h>
#include <esp_attr.h>

// Two-button controls for boards with a forward and backward button.
//
// Mapping:
//   Short press FWD (gpio_fwd)      → DOWN  (next page / move selection down)
//   Short press BWD (gpio_bwd)      → UP    (previous page / move selection up)
//   Both pressed simultaneously     → SELECT
//   Long press FWD  (≥ 1.5 s)      → BACK
//
// Chord detection: when either button is released, the handler checks whether
// the other button is still physically held.  Both buttons must be pressed
// within CHORD_WINDOW_US of each other for the chord to register.
//
// Deep-sleep wakeup: only gpio_bwd is used because gpio_fwd (GPIO 39) is not
// an RTC-capable pin on ESP32-S3 and cannot trigger EXT0 wake.

class DualButtonControls : public ButtonControls
{
private:
  gpio_num_t m_fwd_gpio;
  gpio_num_t m_bwd_gpio;
  ActionCallback_t m_on_action;

  volatile bool    m_fwd_pressed    = false;
  volatile bool    m_bwd_pressed    = false;
  volatile int64_t m_fwd_press_time = 0;
  volatile int64_t m_bwd_press_time = 0;
  // Set when a chord SELECT has been sent; prevents a second action on release
  // of the second button.
  volatile bool    m_chord_active   = false;

  static constexpr int64_t DEBOUNCE_US    =   50000LL; // 50 ms
  static constexpr int64_t LONG_PRESS_US  = 1500000LL; // 1.5 s
  static constexpr int64_t CHORD_WINDOW_US =  80000LL; // 80 ms

  static void IRAM_ATTR fwd_isr(void *arg);
  static void IRAM_ATTR bwd_isr(void *arg);

  void setup_gpio(gpio_num_t pin);

public:
  DualButtonControls(gpio_num_t fwd_gpio, gpio_num_t bwd_gpio,
                     ActionCallback_t on_action);

  bool     did_wake_from_deep_sleep() override;
  UIAction get_deep_sleep_action()    override;
  void     setup_deep_sleep()         override;
};
