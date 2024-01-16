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
//#include "epd_highlevel.h"
#include "epdiy.h"

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


int EPD_HEIGHT = 540;
int EPD_WIDTH = 960;

EpdRotation orientation = EPD_ROT_PORTRAIT;

const char *testsentence = "This is a test";

void displayInfo(void)
{

    
}

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


void buttonPressed(Button2 &b)
{
displayInfo();
}




void setup()
{

/*
    framebuffer = (uint8_t *)ps_calloc(sizeof(uint8_t), displayheight * displaywidth / 2);
    rotatedFramebuffer = (uint8_t *)ps_calloc(sizeof(uint8_t), displayheight * displaywidth / 2);
    if (!framebuffer) {
        Serial.println("alloc memory failed !!!");
        while (1);
    }
    memset(framebuffer, 0xFF, displayheight * displaywidth / 2);
*/
    btn1.setPressedHandler(buttonPressed);
#if defined(T5_47)
    btn2.setPressedHandler(buttonPressed);
    btn3.setPressedHandler(buttonPressed);
#endif

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
