#include "lilyrender.h"
#include "epd_driver.h"


void pushFramebuffer(uint8_t *framebuffer){

int Display_HEIGHT = 540;
int Display_WIDTH = 960;

Rect_t fullscreen = {
    .x = 0,
    .y = 0,
    .width = Display_WIDTH ,
    .height =  Display_HEIGHT,
};

    epd_draw_grayscale_image(fullscreen, framebuffer);
}

void lilygo_init(){
    epd_init();
}
