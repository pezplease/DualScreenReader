#include <esp_log.h>
#include "Board.h"
#include "LilygoT547Plus.h"
#include "controls/TouchControls.h"

#ifdef USE_SPIFFS
#include <SPIFFS.h>
#else
#include <SDCard.h>
#endif

#ifdef BATTERY_ADC_CHANNEL
#include "battery/ADCBattery.h"
#endif

Board *Board::factory()
{
  return new LilygoT547Plus();
}

void Board::start_filesystem()
{
#ifdef USE_SPIFFS
  ESP_LOGI("Board", "Using SPIFFS");
  spiffs = new SPIFFS("/fs");
#else
  ESP_LOGI("Board", "Using SDCard");
  sdcard = new SDCard("/fs", SD_CARD_PIN_NUM_MISO, SD_CARD_PIN_NUM_MOSI, SD_CARD_PIN_NUM_CLK, SD_CARD_PIN_NUM_CS);
#endif
}

void Board::stop_filesystem()
{
#ifdef USE_SPIFFS
  delete spiffs;
  spiffs = nullptr;
#else
  delete sdcard;
  sdcard = nullptr;
#endif
}

Battery *Board::get_battery()
{
#ifdef BATTERY_ADC_CHANNEL
  return new ADCBattery(BATTERY_ADC_CHANNEL);
#else
  return nullptr;
#endif
}

TouchControls *Board::get_touch_controls(Renderer *renderer, xQueueHandle ui_queue)
{
  return new TouchControls();
}
