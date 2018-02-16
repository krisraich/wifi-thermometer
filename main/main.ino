/*
  WiFi Thermometer for ESP32 µC - By Kris 2018

  Features:
    Supports up to 6 Sensors for temperature reading.
    Can operate in different Modes: Power-Saving, WiFi access point with webserver or Bluetooth low energy slave mode.
    Displays temperatures on an E-Paper display or via Web browser.
    Pins are set for LoLin Lite board.

  Docs:
    Wroom Dev Board Pins: https://raw.githubusercontent.com/gojimmypi/ESP32/master/images/myESP32%20DevKitC%20pinout.png
    Wroom Dev Board Doc: https://www.espressif.com/sites/default/files/documentation/esp-wroom-32_datasheet_en.pdf
    ESP32 Datasheet: https://www.espressif.com/sites/default/files/documentation/esp32_datasheet_en.pdf
    LoLin Lite: https://wiki.wemos.cc/products:lolin32:lolin32_lite
    LoLin Lite Schema: https://wiki.wemos.cc/_media/products:lolin32:sch_lolin32_lite_v1.0.0.pdf
    Arduino Pins: https://github.com/espressif/arduino-esp32/blob/master/variants/nodemcu-32s/pins_arduino.h

  Dev Notes:
    Dev Board: Pin6 digital out = Error

  USED REPOSITORIES:
    E-Paper: https://github.com/ZinggJM/GxEPD
    Via Librarie manager: Adafruit GFX


  Program behaviour:
    1. Bootup
      1.A Calibration button pressed enter calibration mode
    2. Load last program state from eeprom and set Interrupt on Buttons (OK/Refresh & Mode)
      2.A: Power save mode (Display temps and go to sleep after specified time)
      2.B: Wifi Mode: start WiFi Hotspot and display temps
      2.C: BT-LE Slave-Mode: Start BT-LE Server, wait for connection. (Not implemented yet)
      2.D: Shutdown Mode: Start Power save mode
    3. Ok/Refresh Button pressed:
      3.A: Menu active: save mode to eeprom and restart
        3.A.a If mode 2.D is selected, show image and go to sleep, wake up on button press
      3.B: Mode 2.A, 2.B or 2.C active: update temp readings
    4. Mode Bottom pressed:
      4.A Menu active: lode next mode (cycle through 2.A - 2.D)
      4.B Program active: do nothing

   ToDo:
    1. Mode switch on button press
    2. Menu open mode
*/

/////////////////
// Defines & Constants & PINS
/////////////////

#define DEBUG true


#define SLEEP_DURATION_SEC  10        /* Time between temp refresh */

#define WIFI_AP_SSID "ESP32"              //Hotspot ID
//#define WIFI_AP_PASSWORD "TestTest123"  //Hotspot PW, Comment for open AP

//onboard button for Wroom Dev Board
#define ON_BOARD_BUTTON 0
#define ON_BOARD_BUTTON_PULLDOWN_MODE true

//onboard LED for Wroom Dev Board
#define ON_BOARD_LED GPIO_NUM_27
#define ON_BOARD_LED_PULLDOWN_MODE true

//Touch button inputs
#define TOUCH_TIME 3 //time in measuring cycles. 1 Cylce 35ms
#define MODE_TOUCH_BUTTON TOUCH_PAD_NUM3 //GPIO 15 See: https://github.com/espressif/arduino-esp32/blob/master/tools/sdk/include/driver/driver/touch_pad.h
#define OK_TOUCH_BUTTON TOUCH_PAD_NUM4 // GPIO 13 Button für refresh

//Pin für LoLin / waveshare 2.9
#define DISPLAY_BUSY 17 // Display BUSY = any GPIO
#define DISPLAY_RST 16 //Display RESET = any GPIO
#define DISPLAY_DC 4 //DATA/COMMAND = any GPIO
#define DISPLAY_CS SS //SPI CHIP SELECT = PIN 5
#define DISPLAY_CLK SCK //SPI CLOCK = Pin 18
#define DISPLAY_DIN MOSI //SPI MOSI (master out slave in) = PIN 23
#define DISABLE_DIAGNOSTIC_OUTPUT     /* Disables Output of Display class*/

#define BATTERY_VOLTAGE_ANALOG_IN  ADC1_CHANNEL_0 //PIN VP

#define uS_TO_S_FACTOR 1000000     /* Conversion factor for micro seconds to seconds */
#define mS_TO_S_FACTOR 1000        /* Conversion factor for milli seconds to seconds */

/////////////////
// LIBS
/////////////////

//scheduler
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

//adc
#include <driver/adc.h>

//touch
#include "driver/touch_pad.h"
#include "soc/rtc_cntl_reg.h"
#include "soc/sens_reg.h"

//display
#include <SPI.h>
#include <GxEPD.h>
#include <GxGDEH029A1/GxGDEH029A1.cpp>
//more fonts: ~/Arduino/libraries/Adafruit_GFX_Library/Fonts
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include <Fonts/FreeMonoBold24pt7b.h>
#include <GxIO/GxIO_SPI/GxIO_SPI.cpp>
#include <GxIO/GxIO.cpp>
#include "logo_mono.h"

//webserver
#include <WiFi.h>

//EEPROM
#include "EEPROM.h"

//handy functions
#define min(a,b) ((a)<(b)?(a):(b));
#define max(a,b) ((a)>(b)?(a):(b));
#define nope()  __asm__("nop\n\t"); 

/////////////////
// Enums and other constants
/////////////////

//Webserver & AP Defaults
const IPAddress Ip(192, 168, 1, 1) ;
const IPAddress NMask(255, 255, 255, 0);

//Analog in for LoLin
const adc1_channel_t ADC_CHANNELS[5] {
  ADC1_CHANNEL_3, //PIN VN
  ADC1_CHANNEL_4, //PIN A1.4/R9/32
  ADC1_CHANNEL_5, //PIN A1.5/R8/33
  ADC1_CHANNEL_6, //PIN A1.6/R4/34
  ADC1_CHANNEL_7  //PIN A1.7/R5/35
};

const touch_pad_t TOUCH_BUTTONS[2] {
  static_cast<touch_pad_t>(OK_TOUCH_BUTTON),
  static_cast<touch_pad_t>(MODE_TOUCH_BUTTON),
};

enum OPERATION_MODE {
  POWER_SAVING = 0,
  WIFI_SERVER = 1,
  BT_LE_SLAVE = 2,
};

enum BLINK_FREQUENCY {
  SLOW = 2,
  NORMAL = 10,
  FAST = 20
};

/////////////////
// Init Display
/////////////////

GxIO_Class io(SPI, DISPLAY_CS, DISPLAY_DC, DISPLAY_RST);  //SPI,SS,DC,RST
GxGDEH029A1 display(io, DISPLAY_RST, DISPLAY_BUSY);  //io,RST,BUSY

/////////////////
// Refresh Task
/////////////////

void refresh_display(void *pvParameter) {
  while(true){
    vTaskDelay(SLEEP_DURATION_SEC * mS_TO_S_FACTOR / portTICK_PERIOD_MS);
    if (DEBUG) Serial.println("Refreshing Display..");
    update_display();
  }
}

/////////////////
// Setup
/////////////////
void setup() {

  setup_led();

  //Initialize serial and wait for port to open:
  if (DEBUG) {
    Serial.begin(115200);
    while (!Serial) {
      delay(10); // wait for serial port to connect. Needed for native USB port only
    }
  }

  led_start_blinking();

  int bootups = setup_deep_sleep();
  setup_adc();

  setup_data_store();

  setup_display();

  //enter only on reset
  if (bootups == 1) {
    calibrate_adcs();
  }


  switch (bootups % 2) {

    case 0:
      display.drawBitmap(gImage_logo_floyd, sizeof(gImage_logo_floyd), GxEPD::bm_invert /* | GxEPD::bm_flip_y */);
      break;
    case 1:
      display.drawBitmap(gImage_logo_floyd, sizeof(gImage_logo_floyd), GxEPD::bm_normal);
      break;
    case 2:
      print_big_text("L00t Boyzz!1", &FreeMonoBold18pt7b);
      break;
  }

  //also needed for deepsleep
  setup_touch();


  //after ini, show temps
  update_display();


  OPERATION_MODE opm = WIFI_SERVER;//get_last_operation_mode();
  if (DEBUG) {
    Serial.println("Starting operation mode: " + String(operation_mode_to_string(opm)));
  }


  switch (opm) {
    case POWER_SAVING:
      start_power_saveing_mode();
      return;
    case WIFI_SERVER:
      setup_webserver();
      xTaskCreate(&refresh_display, "refresh_display", 2048, NULL, 5, NULL);
      return;
    case BT_LE_SLAVE:
      if (DEBUG) Serial.print("Not implemented yet! Starting Power safe mode");
    default: //unknown mode fallthrough
      save_operation_mode(POWER_SAVING);
      ESP.restart();
  }
}


void start_power_saveing_mode() {
  deep_sleep_wake_up_after_time(SLEEP_DURATION_SEC);
  deep_sleep_wake_up_on_touch();
  led_stop_blinking();
  deep_sleep_start();
}

void touch_button_pressed(touch_pad_t pressed_button, bool on_boot) {
  if (on_boot) {
    //esp was woken up by user in power saving mode.. so do nothing
    return;
  }else if(pressed_button == OK_TOUCH_BUTTON){
    // ok button is refresh button
    update_display();
  }else{
    //Mode Button pressed
    show_menu();
  }
  
  
  //Serial.println("touch pin: " + String(pressed_button));
}

/////////////////
// Loop - not executed in power saving mode
/////////////////
void loop() {
   //nope();
  vTaskDelay(100000);
}

