#ifndef BOARD_HAS_PSRAM
#error "Please enable PSRAM !!!"
#endif

#include "Arduino.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <esp_sleep.h>
#include <esp_log.h>
#include <esp_timer.h>

#include "EpubList/Epub.h"
#include "EpubList/EpubList.h"
#include "EpubList/EpubReader.h"
#include "EpubList/EpubToc.h"
#include "boards/Board.h"

static const char *TAG = "main";

typedef enum
{
  SELECTING_EPUB,
  SELECTING_TABLE_CONTENTS,
  READING_EPUB
} UIState;

// State preserved across deep sleep in RTC memory
RTC_NOINIT_ATTR UIState ui_state;
RTC_DATA_ATTR EpubListState epub_list_state;
RTC_DATA_ATTR EpubTocState epub_index_state;

static EpubList *epub_list = nullptr;
static EpubReader *reader = nullptr;
static EpubToc *contents = nullptr;

void handleEpubList(Renderer *renderer, UIAction action, bool needs_redraw);
void handleEpubTableContents(Renderer *renderer, UIAction action, bool needs_redraw);

void handleEpub(Renderer *renderer, UIAction action)
{
  if (!reader)
  {
    reader = new EpubReader(epub_list_state.epub_list[epub_list_state.selected_item], renderer);
    reader->load();
  }
  switch (action)
  {
  case UP:
    reader->prev();
    break;
  case DOWN:
    reader->next();
    break;
  case SELECT:
    ui_state = SELECTING_EPUB;
    renderer->clear_screen();
    delete reader;
    reader = nullptr;
    if (!epub_list)
    {
      epub_list = new EpubList(renderer, epub_list_state);
    }
    handleEpubList(renderer, NONE, true);
    return;
  default:
    break;
  }
  reader->render();
}

void handleEpubTableContents(Renderer *renderer, UIAction action, bool needs_redraw)
{
  if (!contents)
  {
    contents = new EpubToc(epub_list_state.epub_list[epub_list_state.selected_item], epub_index_state, renderer);
    contents->set_needs_redraw();
    contents->load();
  }
  switch (action)
  {
  case UP:
    contents->prev();
    break;
  case DOWN:
    contents->next();
    break;
  case SELECT:
    ui_state = READING_EPUB;
    reader = new EpubReader(epub_list_state.epub_list[epub_list_state.selected_item], renderer);
    reader->set_state_section(contents->get_selected_toc());
    reader->load();
    delete contents;
    contents = nullptr;
    handleEpub(renderer, NONE);
    return;
  default:
    break;
  }
  contents->render();
}

void handleEpubList(Renderer *renderer, UIAction action, bool needs_redraw)
{
  if (!epub_list)
  {
    ESP_LOGI(TAG, "Creating epub list");
    epub_list = new EpubList(renderer, epub_list_state);
    if (epub_list->load("/fs/"))
    {
      ESP_LOGI(TAG, "Epub files loaded");
    }
  }
  if (needs_redraw)
  {
    epub_list->set_needs_redraw();
  }
  switch (action)
  {
  case UP:
    epub_list->prev();
    break;
  case DOWN:
    epub_list->next();
    break;
  case SELECT:
    ui_state = SELECTING_TABLE_CONTENTS;
    contents = new EpubToc(epub_list_state.epub_list[epub_list_state.selected_item], epub_index_state, renderer);
    contents->load();
    contents->set_needs_redraw();
    handleEpubTableContents(renderer, NONE, true);
    return;
  default:
    break;
  }
  epub_list->render();
}

void handleUserInteraction(Renderer *renderer, UIAction ui_action, bool needs_redraw)
{
  switch (ui_state)
  {
  case READING_EPUB:
    handleEpub(renderer, ui_action);
    break;
  case SELECTING_TABLE_CONTENTS:
    handleEpubTableContents(renderer, ui_action, needs_redraw);
    break;
  case SELECTING_EPUB:
  default:
    handleEpubList(renderer, ui_action, needs_redraw);
    break;
  }
}

void draw_battery_level(Renderer *renderer, float voltage, float percentage)
{
  renderer->set_margin_top(0);
  int width = 40;
  int height = 20;
  int margin_right = 5;
  int margin_top = 10;
  int xpos = renderer->get_page_width() - width - margin_right;
  int ypos = margin_top;
  int percent_width = (int)(width * percentage / 100);
  renderer->fill_rect(xpos, ypos, width, height, 255);
  renderer->fill_rect(xpos + width - percent_width, ypos, percent_width, height, 0);
  renderer->draw_rect(xpos, ypos, width, height, 0);
  renderer->fill_rect(xpos - 4, ypos + height / 4, 4, height / 2, 0);
  renderer->set_margin_top(35);
}

void main_task(void *param)
{
  ESP_LOGI(TAG, "Powering up the board");
  Board *board = Board::factory();
  board->power_up();

  ESP_LOGI(TAG, "Creating renderer");
  Renderer *renderer = board->get_renderer();

  ESP_LOGI(TAG, "Starting file system");
  board->start_filesystem();

  Battery *battery = board->get_battery();
  if (battery)
  {
    battery->setup();
  }

  renderer->set_margin_top(35);
  renderer->set_margin_left(10);
  renderer->set_margin_right(10);

  xQueueHandle ui_queue = xQueueCreate(10, sizeof(UIAction));

  ESP_LOGI(TAG, "Setting up controls");
  ButtonControls *button_controls = board->get_button_controls(ui_queue);
  TouchControls *touch_controls = board->get_touch_controls(renderer, ui_queue);

  if (button_controls->did_wake_from_deep_sleep())
  {
    bool hydrate_success = renderer->hydrate();
    UIAction ui_action = button_controls->get_deep_sleep_action();
    handleUserInteraction(renderer, ui_action, !hydrate_success);
  }
  else
  {
    // First boot or reset — initialise ui_state (RTC_NOINIT may hold garbage)
    if (ui_state != SELECTING_EPUB && ui_state != SELECTING_TABLE_CONTENTS && ui_state != READING_EPUB)
    {
      ui_state = SELECTING_EPUB;
    }
    renderer->reset();
    handleUserInteraction(renderer, NONE, true);
  }

  if (battery)
  {
    draw_battery_level(renderer, battery->get_voltage(), battery->get_percentage());
  }
  touch_controls->render(renderer);
  renderer->flush_display();

  int64_t last_user_interaction = esp_timer_get_time();
  // Active for 120 seconds, then deep sleep
  while (esp_timer_get_time() - last_user_interaction < 120LL * 1000 * 1000)
  {
    UIAction ui_action = NONE;
    if (xQueueReceive(ui_queue, &ui_action, pdMS_TO_TICKS(60000)) == pdTRUE)
    {
      if (ui_action != NONE)
      {
        last_user_interaction = esp_timer_get_time();
        touch_controls->renderPressedState(renderer, ui_action);
        handleUserInteraction(renderer, ui_action, false);
        touch_controls->render(renderer);
        // Drain any presses that accumulated during the slow render so they
        // don't cascade the UI state machine through multiple screens at once.
        UIAction stale;
        while (xQueueReceive(ui_queue, &stale, 0) == pdTRUE) {}
      }
    }
    if (battery)
    {
      draw_battery_level(renderer, battery->get_voltage(), battery->get_percentage());
    }
    renderer->flush_display();
  }

  ESP_LOGI(TAG, "Saving state and entering deep sleep");
  renderer->dehydrate();
  board->stop_filesystem();
  board->prepare_to_sleep();
  button_controls->setup_deep_sleep();
  vTaskDelay(pdMS_TO_TICKS(500));
  esp_deep_sleep_start();
}

void setup()
{
  Serial.begin(115200);

  esp_log_level_set("main", ESP_LOG_INFO);
  esp_log_level_set("EPUB", ESP_LOG_INFO);
  esp_log_level_set("PUBLIST", ESP_LOG_INFO);
  esp_log_level_set("ZIP", ESP_LOG_INFO);
  esp_log_level_set("JPG", ESP_LOG_INFO);

  ESP_LOGI(TAG, "Free heap before main task: %d", esp_get_free_heap_size());
  xTaskCreatePinnedToCore(main_task, "main_task", 32768, NULL, 1, NULL, 1);
}

void loop()
{
  // The main logic runs in main_task; keep the Arduino loop idle.
  delay(1000);
}
