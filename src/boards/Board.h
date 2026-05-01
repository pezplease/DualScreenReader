#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <stdint.h>
#include "controls/ButtonControls.h"
#include "controls/TouchControls.h"
#include "battery/Battery.h"

class SDCard;
class SPIFFS;
class Renderer;

class Board
{
protected:
#ifdef USE_SPIFFS
  SPIFFS *spiffs = nullptr;
#else
  SDCard *sdcard = nullptr;
#endif

public:
  virtual void power_up() = 0;
  virtual void prepare_to_sleep() = 0;
  virtual Renderer *get_renderer() = 0;
  virtual void start_filesystem();
  virtual void stop_filesystem();
  virtual Battery *get_battery();
  virtual ButtonControls *get_button_controls(xQueueHandle ui_queue) = 0;
  virtual TouchControls *get_touch_controls(Renderer *renderer, xQueueHandle ui_queue);

  static Board *factory();
};
