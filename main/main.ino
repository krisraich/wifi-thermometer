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
    Arduino Pins: https://github.com/espressif/arduino-esp32/blob/master/variants/nodemcu-32s/pins_arduino.h

  Dev Notes:
    Dev Board: Pin6 digital out = Error

  USED REPOSITORIES:
    E-Paper: https://github.com/ZinggJM/GxEPD 
    WebServer: https://github.com/me-no-dev/ESPAsyncWebServer & https://github.com/me-no-dev/AsyncTCP
    Via Librarie manager: Adafruit GFX, ArduinoJson


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
*/


/////////////////
// LIBS
/////////////////

//adc
#include <driver/adc.h>

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
#include "logo_4c.h"
#include "logo_mono.h"

//webserver
#include <AsyncTCP.h>
#include <AsyncEventSource.h>
#include <AsyncJson.h>
#include <AsyncWebSocket.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFSEditor.h>
#include <StringArray.h>
#include <WebAuthentication.h>
#include <WebHandlerImpl.h>
#include <WebResponseImpl.h>
#include <WiFi.h>

//EEPROM
#include "EEPROM.h"

/////////////////
// Defines & Constants
/////////////////

#define DEBUG true

#define SLEEP_DURATION_SEC  20        /* Time ESP32 will go to sleep (in seconds) */
#define BUFFER_TIME_EXT_WAKE_UP 500   /* Time ESP32 will wait befor next external wakeup (in milliseconds)*/

#define uS_TO_S_FACTOR 1000000        /* Conversion factor for micro seconds to seconds */

/////////////////
// Pin Setup
/////////////////

//onboard button for Wroom Dev Board
#define ON_BOARD_BUTTON 0
#define ON_BOARD_BUTTON_PULLDOWN_MODE true

//onboard LED for Wroom Dev Board
#define ON_BOARD_LED GPIO_NUM_27
#define ON_BOARD_LED_PULLDOWN_MODE true


//Touch button inputs
#define TOUCH_THRESHOLD 40 //Greater the value, more the sensitivity
#define MODE_TOUCH_BUTTON TOUCH_PAD_NUM1 // --> RTC_GPIO16 / TOUCH 6 / GPIO0 
#define OK_TOUCH_BUTTON TOUCH_PAD_NUM2 // --> RTC_GPIO17 / TOUCH 7 / GPIO2 


//Pin für LoLin / waveshare 2.9
#define DISPLAY_BUSY 17 // Display BUSY = any GPIO
#define DISPLAY_RST 16 //Display RESET = any GPIO
#define DISPLAY_DC 4 //DATA/COMMAND = any GPIO
#define DISPLAY_CS SS //SPI CHIP SELECT = PIN 5
#define DISPLAY_CLK SCK //SPI CLOCK = Pin 18
#define DISPLAY_DIN MOSI //SPI MOSI (master out slave in) = PIN 23

//Analog in for LoLin
const adc1_channel_t ADC_CHANNELS[6] {
  ADC1_CHANNEL_0, //PIN VP
  ADC1_CHANNEL_3, //PIN VN
  ADC1_CHANNEL_4, //PIN A1.4/R9/32
  ADC1_CHANNEL_5, //PIN A1.5/R8/33
  ADC1_CHANNEL_6, //PIN A1.6/R4/34
  ADC1_CHANNEL_7  //PIN A1.7/R5/35
};


enum OPERATION_MODE{
  POWER_SAVING = 0,
  WIFI_SERVER = 1,
  BT_LE_SLAVE = 2,
};

enum BLINK_FREQUENCY{
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
// Interrupt
/////////////////
void IRAM_ATTR mode_button_pressed() {
  if (DEBUG) Serial.println("mode interrupt");
  delay(100);
}
void IRAM_ATTR ok_button_pressed() {
  if (DEBUG) Serial.println("ok/refresh interrupt");
  delay(100);
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
      ; // wait for serial port to connect. Needed for native USB port only
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


  switch(bootups % 3){
     case 0:
      print_big_text("L00t Boyzz!1", &FreeMonoBold18pt7b);
      break;
    case 1:
      display.drawBitmap(gImage_logo_mono, sizeof(gImage_logo_mono), GxEPD::bm_invert | GxEPD::bm_flip_y);
      break;
    case 2:
      display.drawBitmap(gImage_logo_mono, sizeof(gImage_logo_mono), GxEPD::bm_normal | GxEPD::bm_flip_y);
      break;
  }

  setup_touch();


  OPERATION_MODE opm = get_last_operation_mode();
  if (DEBUG){
    Serial.print("Starting operation mode: ");
    Serial.println(operation_mode_to_string(opm));
  }
  
  /*
    gpio_set_pull_mode(MODE_SWITCH_PIN, MODE_SWITCH_ON_HIGH ? GPIO_PULLDOWN_ONLY : GPIO_PULLUP_ONLY);
    pinMode(MODE_SWITCH_PIN, MODE_SWITCH_ON_HIGH ? INPUT : INPUT_PULLUP); // MODE SWITCHER INPUT / INPUT_PULLUP for High
    attachInterrupt(digitalPinToInterrupt(MODE_SWITCH_PIN), handleInterrupt, CHANGE);
  */


  switch(opm){
    case POWER_SAVING:
      start_power_saveing_mode();
      return;
    case WIFI_SERVER:
      setup_webserver();
      //TODO: remove
      save_operation_mode(POWER_SAVING);
      return; //enter Loop?
    case BT_LE_SLAVE:
      if (DEBUG) Serial.print("Not implemented yet! Starting Power safe mode");
    default: //unknown mode fallthrough
      save_operation_mode(POWER_SAVING);
      ESP.restart();
    
  }
  
}


void start_power_saveing_mode(){
  if (DEBUG) Serial.println("Energy Saving Mode");

  deep_sleep_wake_up_after_time(SLEEP_DURATION_SEC);
  deep_sleep_wake_up_on_touch();

  display_temps_on_display();

  led_stop_blinking();
  deep_sleep_start();
}


/////////////////
// Loop - not executed in power saving mode
/////////////////
void loop() {

  Serial.println("Wifi Active...");
  display_temps_on_display();
  delay(5000);
  

}


//schlauer kris vergisst den DataStore hinzuzufügen. Dummy Code...
void setup_data_store(){}
OPERATION_MODE get_last_operation_mode(){
  return POWER_SAVING;
}
void save_operation_mode(OPERATION_MODE dummy){}


