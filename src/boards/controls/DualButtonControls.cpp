#include "DualButtonControls.h"
#include <driver/gpio.h>
#include <driver/rtc_io.h>
#include <esp_sleep.h>
#include <esp_timer.h>
#include <esp_log.h>

static const char *TAG = "DualBtn";

// ---------------------------------------------------------------------------
// ISR handlers — called on both press and release (ANYEDGE).
// gpio_get_level determines which edge fired.
// ---------------------------------------------------------------------------

void IRAM_ATTR DualButtonControls::fwd_isr(void *arg)
{
  auto *self = static_cast<DualButtonControls *>(arg);
  int level = gpio_get_level(self->m_fwd_gpio);

  if (level == 0)
  {
    // Pressed (active-low)
    self->m_fwd_pressed    = true;
    self->m_fwd_press_time = esp_timer_get_time();
    return;
  }

  // Released
  if (!self->m_fwd_pressed) return;
  self->m_fwd_pressed = false;

  int64_t duration = esp_timer_get_time() - self->m_fwd_press_time;
  if (duration < DEBOUNCE_US) return;

  // If this release is the second half of a chord, just clear the flag.
  if (self->m_chord_active)
  {
    self->m_chord_active = false;
    return;
  }

  // Check whether BWD is still held — if so this is a chord.
  bool bwd_held    = (gpio_get_level(self->m_bwd_gpio) == 0);
  bool bwd_recent  = self->m_bwd_pressed ||
                     ((esp_timer_get_time() - self->m_bwd_press_time) < CHORD_WINDOW_US);

  if (bwd_held || bwd_recent)
  {
    self->m_chord_active = true;
    UIAction action = UIAction::SELECT;
    self->m_on_action(action);
    return;
  }

  // Single FWD press: long-press → BACK, short press → DOWN
  UIAction action = (duration >= LONG_PRESS_US) ? UIAction::BACK : UIAction::DOWN;
  self->m_on_action(action);
}

void IRAM_ATTR DualButtonControls::bwd_isr(void *arg)
{
  auto *self = static_cast<DualButtonControls *>(arg);
  int level = gpio_get_level(self->m_bwd_gpio);

  if (level == 0)
  {
    // Pressed
    self->m_bwd_pressed    = true;
    self->m_bwd_press_time = esp_timer_get_time();
    return;
  }

  // Released
  if (!self->m_bwd_pressed) return;
  self->m_bwd_pressed = false;

  int64_t duration = esp_timer_get_time() - self->m_bwd_press_time;
  if (duration < DEBOUNCE_US) return;

  if (self->m_chord_active)
  {
    self->m_chord_active = false;
    return;
  }

  bool fwd_held   = (gpio_get_level(self->m_fwd_gpio) == 0);
  bool fwd_recent = self->m_fwd_pressed ||
                    ((esp_timer_get_time() - self->m_fwd_press_time) < CHORD_WINDOW_US);

  if (fwd_held || fwd_recent)
  {
    self->m_chord_active = true;
    UIAction action = UIAction::SELECT;
    self->m_on_action(action);
    return;
  }

  UIAction action = UIAction::UP;
  self->m_on_action(action);
}

// ---------------------------------------------------------------------------
// Setup
// ---------------------------------------------------------------------------

void DualButtonControls::setup_gpio(gpio_num_t pin)
{
  gpio_set_direction(pin, GPIO_MODE_INPUT);
  gpio_set_pull_mode(pin, GPIO_PULLUP_ONLY);
  gpio_set_intr_type(pin, GPIO_INTR_ANYEDGE);
  gpio_intr_enable(pin);
}

DualButtonControls::DualButtonControls(gpio_num_t fwd_gpio, gpio_num_t bwd_gpio,
                                       ActionCallback_t on_action)
    : m_fwd_gpio(fwd_gpio), m_bwd_gpio(bwd_gpio), m_on_action(on_action)
{
  gpio_install_isr_service(0);
  setup_gpio(m_fwd_gpio);
  setup_gpio(m_bwd_gpio);
  gpio_isr_handler_add(m_fwd_gpio, fwd_isr, this);
  gpio_isr_handler_add(m_bwd_gpio, bwd_isr, this);
  ESP_LOGI(TAG, "Dual-button controls on GPIO %d (fwd) and %d (bwd)",
           (int)m_fwd_gpio, (int)m_bwd_gpio);
}

// ---------------------------------------------------------------------------
// Deep sleep
// ---------------------------------------------------------------------------

bool DualButtonControls::did_wake_from_deep_sleep()
{
  return esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_EXT0;
}

UIAction DualButtonControls::get_deep_sleep_action()
{
  return UIAction::NONE;
}

void DualButtonControls::setup_deep_sleep()
{
  // Only GPIO 0-21 are RTC-capable on ESP32-S3; use the backward button (bwd)
  // for wakeup.  The forward button (GPIO 39) cannot trigger EXT0.
  rtc_gpio_init(m_bwd_gpio);
  rtc_gpio_set_direction(m_bwd_gpio, RTC_GPIO_MODE_INPUT_ONLY);
  rtc_gpio_pullup_en(m_bwd_gpio);
  esp_sleep_enable_ext0_wakeup(m_bwd_gpio, 0);
  ESP_LOGI(TAG, "Deep sleep wakeup on GPIO %d (bwd)", (int)m_bwd_gpio);
}
