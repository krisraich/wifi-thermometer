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

   Bugs:
    1. Sometimes AP starts with IP: 192.168.1.4 instead of 192.168.1.1
    2. Guru Meditation Error when switching modes: mostly WIFI_SERVER to POWER_SAVING (Cache disabled but cached memory region accessed)
*/

/////////////////
// Defines & Constants & PINS
/////////////////

#define DEBUG true

#define SLEEP_DURATION_SEC  10        /* Time between temp refresh */

#define TEMPERATUR_HISTORY_SAMPLE_RATIO  2 /* SLEEP_DURATION_SEC * TEMPERATUR_HISTORY_SAMPLE_RATIO * 296 = Time span of history (in sec)  */

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

#define MIN_TIME_BETWEEN_REFRESH  1000 //zeit die vergehen muss bevor der benutzer die temperaturen aktualisieren kann

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
const adc1_channel_t ADC_CHANNELS[] {
  ADC1_CHANNEL_3, //PIN VN
  ADC1_CHANNEL_4, //PIN A1.4/R9/32
  ADC1_CHANNEL_5, //PIN A1.5/R8/33
  ADC1_CHANNEL_6, //PIN A1.6/R4/34
  ADC1_CHANNEL_7  //PIN A1.7/R5/35
};

const touch_pad_t TOUCH_BUTTONS[] {
  static_cast<touch_pad_t>(OK_TOUCH_BUTTON),
  static_cast<touch_pad_t>(MODE_TOUCH_BUTTON),
};

enum OPERATION_MODE {
  POWER_SAVING = 0,
  WIFI_SERVER = 1,
  BT_LE_SLAVE = 2,
  SHUTDOWN = 99
};

enum BLINK_FREQUENCY {
  SLOW = 2,
  NORMAL = 10,
  FAST = 20
};

bool menu_open = false;
unsigned long last_refresh = 0;
OPERATION_MODE current_operation_mode;
OPERATION_MODE selected_operation_mode;

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
    //only refresh when user hasn't refreshed it
    if(!menu_open && millis() - last_refresh > SLEEP_DURATION_SEC * mS_TO_S_FACTOR){
      if (DEBUG) Serial.println("Auto refreshing temps: ");
      update_display();
    }
    //else if (DEBUG) Serial.println("Menu is open, don't refresh display");

    record_temperatures();
    vTaskDelay(SLEEP_DURATION_SEC * mS_TO_S_FACTOR / portTICK_PERIOD_MS);
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

  setup_recorder();

  setup_display();

  //enter only on reset
  if (bootups == 1) {
    calibrate_adcs();
  }

  //also needed for deepsleep
  setup_touch();

  current_operation_mode = get_last_operation_mode();
  selected_operation_mode = get_last_operation_mode(); //using current_operation_mode would copy the reference... 
  
  if (DEBUG) Serial.println("Starting operation mode: " + String(operation_mode_to_string(current_operation_mode)));
 
  switch (current_operation_mode) {
    //fall back into POWER_SAVING
    case BT_LE_SLAVE:
      if (DEBUG) Serial.print("Not implemented yet! Starting Power safe mode");
    default:
      //set default
      save_operation_mode(POWER_SAVING);
    case POWER_SAVING:
      if(!menu_open){
        //after ini, show temps. except when menu is open
        update_display();
        start_power_saveing_mode();
      }
      return;
    case WIFI_SERVER:
      start_wifi_mode();
      return;
  }
}

void start_wifi_mode(){
  setup_webserver();
  xTaskCreate(&refresh_display, "refresh_display", 2048, NULL, 5, NULL);
}

void start_power_saveing_mode() {
  deep_sleep_wake_up_after_time(SLEEP_DURATION_SEC);
  deep_sleep_wake_up_on_touch();
  led_stop_blinking();
  deep_sleep_start();
}

void default_procedure_on_error(){
  if(DEBUG) Serial.println("Default procedure startet.. rebooting into POWER_SAVING");
  save_operation_mode(POWER_SAVING);
  //delay(1000);
  ESP.restart();
}

void touch_button_pressed(touch_pad_t pressed_button, bool on_boot) {

  if(DEBUG) Serial.println("Touch input: " + String(pressed_button == OK_TOUCH_BUTTON ? "ok/refresh" : "mode"));
  
  if (on_boot && pressed_button == OK_TOUCH_BUTTON) {
    //esp was woken up by user in power saving mode.,
    //so do nothing and just refresh the temps
  }else if (pressed_button == MODE_TOUCH_BUTTON) {
    
    //when esp was woken up by user pressing the mode button,
    //always show menu, becaus it'll never go to sleep when the menu is open
    //since the menu is also called when esp is awake, just ignor boot option and show menu
    if(menu_open){
      if(DEBUG) Serial.print("switch mode from " + String(operation_mode_to_string(selected_operation_mode)));
      selected_operation_mode = cycle_through_modes(selected_operation_mode);
      if(DEBUG) Serial.println(" to " + String(operation_mode_to_string(selected_operation_mode)));
    }else{
      menu_open = true;
    }
    show_menu(selected_operation_mode);

  }else if(pressed_button == OK_TOUCH_BUTTON){
    // ok button is refresh button when menu is active
    if(menu_open){
      //select menu ....
      if(DEBUG) Serial.println("select mode! Going from " + String(operation_mode_to_string(current_operation_mode)) + " to " + String(operation_mode_to_string(selected_operation_mode)));
      menu_open = false;
      
      switch(selected_operation_mode){
        
        //only when mode was active and user waked up esp
        case POWER_SAVING:
          save_operation_mode(POWER_SAVING);
          switch(current_operation_mode){
            //going from POWER_SAVING to POWER_SAVING
            case POWER_SAVING:
              start_power_saveing_mode(); //menu is closed.. now go to sleep
              break;
            //going from WIFI_SERVER to POWER_SAVING
            case WIFI_SERVER:
              
              //works?
              esp_sleep_enable_timer_wakeup(200000);
              esp_deep_sleep_start();
              
              //ESP.restart(); //needs restart to turn off wifi
              break;
            //default: mode has been saved and it should load next boot
            default: ESP.restart();
          }
          break;

        
        case WIFI_SERVER:
          save_operation_mode(WIFI_SERVER);
          switch(current_operation_mode){
            //going from POWER_SAVING to WIFI_SERVER
            case POWER_SAVING:
              current_operation_mode = WIFI_SERVER;
              start_wifi_mode();
              break;
            //going from WIFI_SERVER to WIFI_SERVER
            case WIFI_SERVER:
              if (DEBUG) Serial.println("nothing todo...");
              break;
            //default: mode has been saved and it should load next boot
            default: ESP.restart();
          }
          break;

        case SHUTDOWN:
          //save_operation_mode(POWER_SAVING);
          show_shutdown();
          deep_sleep_wake_up_after_time(2147483647); //68 years. should be enough
          deep_sleep_start();
          break;
        
        //default fallback
        default: default_procedure_on_error();
      }
              
    }else if(millis() - last_refresh > MIN_TIME_BETWEEN_REFRESH){
      if(DEBUG) Serial.println("user refresh temps: ");
      update_display();
    }

  }else{
    if(DEBUG) Serial.println("You shouldn't see this...");
  }
  
}

/////////////////
// Loop - not executed in power saving mode
/////////////////
void loop() {
   //nope();
  vTaskDelay(3 * mS_TO_S_FACTOR / portTICK_PERIOD_MS);
}

