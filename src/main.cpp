#ifndef BOARD_HAS_PSRAM
#error "Please enable PSRAM !!!"
#endif

#include "Arduino.h"
#include "Button2.h"
#include "pins.h"

// esp32 sdk imports
#include "esp_heap_caps.h"
#include "esp_log.h"

// epd
#include "epd_highlevel.h"
#include "epdiy.h"

// battery
#include <driver/adc.h>
#include "esp_adc_cal.h"

// deepsleep
#include "esp_sleep.h"

// font
#include "Firasans.h"



void buttonPressed(Button2 &b);
void displayInfo(void);

Button2 btn1(BUTTON_1);
#if defined(T5_47)
Button2 btn2(BUTTON_2);
Button2 btn3(BUTTON_3);
#endif

uint8_t *framebuffer;
uint8_t *rotatedFramebuffer;

int vref = 1100;
int cursor_x = 20;
int cursor_y = 60;
int state = 0;

int displayheight = 540;
int displaywidth = 960;

const char *testsentence = "This is a test";

void displayInfo(void)
{

    
}
void buttonPressed(Button2 &b)
{
    displayInfo();
}


void setup()
{
    Serial.begin(115200);


    framebuffer = (uint8_t *)ps_calloc(sizeof(uint8_t), displayheight * displaywidth / 2);
    rotatedFramebuffer = (uint8_t *)ps_calloc(sizeof(uint8_t), displayheight * displaywidth / 2);
    if (!framebuffer) {
        Serial.println("alloc memory failed !!!");
        while (1);
    }
    memset(framebuffer, 0xFF, displayheight * displaywidth / 2);

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
