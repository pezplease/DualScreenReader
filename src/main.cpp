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

//actual dimensions of the screen (lilygo 4.7 is height 540 width 960)
int Display_HEIGHT = 540;
int Display_WIDTH = 960;

int Buffer_HEIGHT = 960;
int Buffer_WIDTH = 540;
uint8_t *fb;
uint8_t *rot_fb;

Rect_t fullscreen = {
    .x = 0,
    .y = 0,
    .width = Display_WIDTH ,
    .height =  Display_HEIGHT,
};
//EpdRect allarea = {.x = 0, .y = 0, .width = epd_width(), .height = epd_height()};


EpdRotation orientation = EPD_ROT_LANDSCAPE;

const char *testsentence = "This is an E-reader By Tatman Bernard";

void rotateFramebuffer(uint8_t *normalbuffer, uint8_t *rotatedbuffer){


for (int x = 0; x < Display_WIDTH; x++) {
        for (int y = 0; y < Display_HEIGHT; y++) {
            // Calculate the index in the input buffer
            int inputIndex = y * Display_WIDTH + x;

            // Calculate the index in the output buffer after rotation
            int outputIndex = (Display_WIDTH - x - 1) * Display_HEIGHT + y;

            // Copy the pixel data from input to output
            rotatedbuffer[outputIndex] = normalbuffer[inputIndex];
        }
    }



/*
for (int x = 0; x < Buffer_WIDTH; x++) {
    for (int y = 0; y < Buffer_HEIGHT; y++) {
        int srcIndex = y * (Buffer_WIDTH / 2) + (x / 8);
        int srcBit = x % 8;
        int destIndex = (Buffer_WIDTH - x - 1) * (Buffer_HEIGHT / 2) + (y / 8);
        int destBit = y % 8;
       // Extract the pixel from the source and set it in the destination
        uint8_t pixel = (normalbuffer[srcIndex] >> srcBit) & 0x01;
        rotatedbuffer[destIndex] |= (pixel << destBit);
    }
}
*/

/*
for (int x = 0; x < Buffer_WIDTH; x++) {
        for (int y = 0; y < Buffer_HEIGHT; y++){
            rotatedbuffer[y * Buffer_WIDTH + x] = normalbuffer[(Buffer_WIDTH - x - 1) * Buffer_HEIGHT + y];
        }
    }

for (int y = 0; y < Display_HEIGHT; y++) {
    for (int x = 0; x < Display_WIDTH; x++){
        int originalIndex = y * Display_WIDTH + x;

        int rotatedIndex = x *Display_HEIGHT + y;

        rotatedbuffer[rotatedIndex] = normalbuffer[originalIndex];
    }
*/
}


void displayInfo(void)
{
epd_poweron();
epd_clear();

    int cursor_x = Buffer_HEIGHT / 2;
    int cursor_y = Buffer_WIDTH / 2;
    if (orientation == EPD_ROT_PORTRAIT) {
        // height and width switched here because portrait mode
        cursor_x = Buffer_WIDTH / 2;
        cursor_y = Buffer_HEIGHT / 2;
    }
    EpdFontProperties font_props = epd_font_properties_default();
    font_props.flags = EPD_DRAW_ALIGN_CENTER;
    
    //epd_write_string(&FiraSans_12, testsentence, &cursor_x, &cursor_y, fb, &font_props);
epd_write_default(&FiraSans_12, testsentence, &cursor_x, &cursor_y, fb);
    //epd_draw_hline(50,200,300,0, fb);
//rotateFramebuffer(fb, rot_fb);

//epdiy_draw_rotated_image(allarea, fb, fb);
//epd_draw_rotated_transparent_image(allarea, fb, fb,null);
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
displayInfo();
}


void setup()
{
    Serial.begin(115200);
   
    epd_init();
    //epd_set_rotation(EPD_ROT_PORTRAIT);
    
    fb = (uint8_t *)ps_calloc(sizeof(uint8_t), Display_HEIGHT * Display_WIDTH / 2);
    if (!fb) {
        Serial.println("alloc memory failed");
        while (1);
    }
    rot_fb = (uint8_t *)ps_calloc(sizeof(uint8_t), Display_HEIGHT * Display_WIDTH / 2);
     if (!rot_fb) {
        Serial.println("alloc rot memory failed");
        while (1);
    }
    memset(fb, 0xFF, Display_HEIGHT * Display_WIDTH / 2);
    memset(rot_fb, 0xFF, Display_HEIGHT * Display_WIDTH / 2);
    //epd_poweron();
   


    btn1.setPressedHandler(buttonPressed);
    //displayInfo();
   
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
