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

EpdiyHighlevelState hl;
// ambient temperature around device
int temperature = 20;
uint8_t* fb;
enum EpdDrawError err;

EpdRotation orientation = EPD_ROT_PORTRAIT;

int vref = 1100;



void correct_adc_reference() {
    esp_adc_cal_characteristics_t adc_chars;
    esp_adc_cal_value_t val_type =
        esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, &adc_chars);
    if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
        Serial.printf("eFuse Vref:%u mV", adc_chars.vref);
        vref = adc_chars.vref;
    }
}

double_t get_battery_percentage() {
    // When reading the battery voltage, POWER_EN must be turned on
    epd_poweron();
    delay(50);

    Serial.println(epd_ambient_temperature());

    uint16_t v = analogRead(BATT_PIN);
    Serial.print("Battery analogRead value is");
    Serial.println(v);
    double_t battery_voltage = ((double_t)v / 4095.0) * 2.0 * 3.3 * (vref / 1000.0);

    // Better formula needed I suppose
    // experimental super simple percent estimate no lookup anything just divide by 100
    double_t percent_experiment = ((battery_voltage - 3.7) / 0.5) * 100;

    // cap out battery at 100%
    // on charging it spikes higher
    if (percent_experiment > 100) {
        percent_experiment = 100;
    }

    String voltage = "Battery Voltage :" + String(battery_voltage) + "V which is around " +
                     String(percent_experiment) + "%";
    Serial.println(voltage);

    epd_poweroff();
    delay(50);

    return percent_experiment;
}

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
    display_center_message(testsentence);
}




void setup()
{
    Serial.begin(115200);
     correct_adc_reference();

    epd_init(&epd_board_lilygo_t5_47, &ED097TC2, EPD_LUT_64K);
    epd_set_vcom(1560);
    hl = epd_hl_init(WAVEFORM);
    epd_set_rotation(orientation);
    fb = epd_hl_get_framebuffer(&hl);
    epd_clear();

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
