#ifndef BOARD_HAS_PSRAM
#error "Please enable PSRAM !!!"
#endif

#include "Arduino.h"
#include "Button2.h"
#include "pins.h"

// esp32 sdk imports
//#include "esp_heap_caps.h"
//#include "esp_log.h"

// epd
#include "epd_highlevel.h"
#include "epdiy.h"

//lilygo epd drivers
#include "epd_driver.h"


// battery
//#include <driver/adc.h>
//#include "esp_adc_cal.h"

// deepsleep
//#include "esp_sleep.h"

// font
#include "Firasans.h"

#define WAVEFORM EPD_BUILTIN_WAVEFORM

void buttonPressed(Button2 &b);
void displayInfo(void);

Button2 btn1(BUTTON_1);
#if defined(T5_47)
Button2 btn2(BUTTON_2);
Button2 btn3(BUTTON_3);
#endif

//uint8_t *framebuffer;
//uint8_t *rotatedFramebuffer;


int Display_HEIGHT = 540;
int Display_WIDTH = 960;

uint8_t* fb;

Rect_t fullscreen = {
    .x = 0,
    .y = 0,
    .width = Display_WIDTH ,
    .height =  Display_HEIGHT,
};

EpdRotation orientation = EPD_ROT_PORTRAIT;

const char *testsentence = "This is a test";

void displayInfo(void)
{
epd_poweron();
epd_clear();

    int cursor_x = Display_HEIGHT / 2;
    int cursor_y = Display_WIDTH / 2;
    if (orientation == EPD_ROT_PORTRAIT) {
        // height and width switched here because portrait mode
        cursor_x = Display_WIDTH / 2;
        cursor_y = Display_HEIGHT / 2;
    }
    EpdFontProperties font_props = epd_font_properties_default();
    font_props.flags = EPD_DRAW_ALIGN_CENTER;
    epd_write_string(&FiraSans_12, testsentence, &cursor_x, &cursor_y, fb, &font_props);

epd_draw_grayscale_image(fullscreen, fb);
epd_poweroff();
delay(3000);

}
/*
void display_center_message(const char* text) {
    // first set full screen to white
    epd_hl_set_all_white(&hl);

    int cursor_x = EPD_WIDTH / 2;
    int cursor_y = EPD_HEIGHT / 2;
    if (orientation == EPD_ROT_PORTRAIT) {
        // height and width switched here because portrait mode
        cursor_x = EPD_HEIGHT / 2;
        cursor_y = EPD_WIDTH / 2;
    }
    EpdFontProperties font_props = epd_font_properties_default();
    font_props.flags = EPD_DRAW_ALIGN_CENTER;
    epd_write_string(&FiraSans_12, text, &cursor_x, &cursor_y, fb, &font_props);

    epd_poweron();
    err = epd_hl_update_screen(&hl, MODE_GC16, temperature);
    delay(500);
    epd_poweroff();
    delay(1000);
}
*/

void buttonPressed(Button2 &b)
{
//displayInfo();
}


void setup()
{
    Serial.begin(115200);

    epd_init();

    fb = (uint8_t *)ps_calloc(sizeof(uint8_t), Display_HEIGHT * Display_WIDTH / 2);
    if (!fb) {
        Serial.println("alloc memory failed !!!");
        while (1);
    }
    memset(fb, 0xFF, Display_HEIGHT * Display_WIDTH / 2);

    //epd_poweron();
   


    btn1.setPressedHandler(buttonPressed);
    displayInfo();
   
}

void loop()
{
 
    btn1.loop();
#if defined(T5_47_PLUS)
    delay(20);

#endif
#if defined(T5_47)
    btn2.loop();
    btn3.loop();
#endif
}
