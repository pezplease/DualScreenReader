#include "SingleButtonControls.h"
#include <esp_sleep.h>
#include <driver/rtc_io.h>
#include <driver/gpio.h>
#include <esp_log.h>

static const char *TAG = "SingleBtn";

SingleButtonControls::SingleButtonControls(gpio_num_t gpio, ActionCallback_t on_action)
    : m_gpio(gpio), m_on_action(on_action)
{
  gpio_install_isr_service(0);
  // The button fires SELECT
  m_button = new GPIOButton(
      gpio, 0 /* active low */,
      [this]() { this->m_on_action(UIAction::SELECT); },
      [this]() { this->m_on_action(UIAction::BACK); });
}

bool SingleButtonControls::did_wake_from_deep_sleep()
{
  auto cause = esp_sleep_get_wakeup_cause();
  if (cause == ESP_SLEEP_WAKEUP_EXT0)
  {
    ESP_LOGI(TAG, "Woke from deep sleep via button");
    return true;
  }
  return false;
}

UIAction SingleButtonControls::get_deep_sleep_action()
{
  // Wake-up should restore the current page, not advance it.
  // The user can press again to turn the page.
  return UIAction::NONE;
}

void SingleButtonControls::setup_deep_sleep()
{
  // Configure the button GPIO as an RTC input so it can wake the device
  rtc_gpio_init(m_gpio);
  rtc_gpio_set_direction(m_gpio, RTC_GPIO_MODE_INPUT_ONLY);
  rtc_gpio_pullup_en(m_gpio);
  // Wake on low level (button active low)
  esp_sleep_enable_ext0_wakeup(m_gpio, 0);
}
