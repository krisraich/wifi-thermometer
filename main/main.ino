/*
  By Kris
  http://www.instructables.com/id/IOT-Made-Simple-Playing-With-the-ESP32-on-Arduino-/
  https://techtutorialsx.com/2017/05/09/esp32-running-code-on-a-specific-core/
  https://raw.githubusercontent.com/gojimmypi/ESP32/master/images/myESP32%20DevKitC%20pinout.png

  Dev Baord: Pin6 digital out = Error

  USED REPOSITORIES:
    E-Paper: https://github.com/ZinggJM/GxEPD (not used: https://github.com/loboris/ESP32_ePaper_example)
    WebServer: https://github.com/me-no-dev/ESPAsyncWebServer & https://github.com/me-no-dev/AsyncTCP
    Via Bib-Manager: Adafruit GFX, ArduinoJson


  Program behaviour:
    1. Bootup
      1.A Calibration button pressed enter calibration mode
    2. Load last programm state from eeprom and set Interrup on Buttons (OK/Refresh & Mode)
      2.A: Power save mode (Display temps and go to sleep after specified time)
      2.B: Wifi Mode: start WiFi Hotspot and display temps
      2.C: BT-LE Slave-Mode: Start BT-LE Server, wait for connection.
      2.D: Shutdown Mode: Start Power save mode
    3. Ok/Refresh Button pressed:
      3.A: Menue active: save mode to eeprom and restart
        3.A.a If mode 2.D is selcetd, show image and go to sleep, wake up on button press
      3.B: Mode 2.A, 2.B or 2.C active: update temp readings
    4. Mode Botton pressed:
      4.A Menue active: lode next mode (cycle throu 2.A - 2.D)
      4.B Porgam active: do nothing


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

/////////////////
// Defines & Constants
/////////////////

#define DEBUG true

#define SLEEP_DURATION_SEC  20       /* Time ESP32 will go to sleep (in seconds) */
#define BUFFER_TIME_EXT_WAKE_UP 500 /* Time ESP32 will wait befor next external wakeup (in milliseconds)*/



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
const adc1_channel_t adc_channels[6] {
  ADC1_CHANNEL_0, //PIN VP
  ADC1_CHANNEL_3, //PIN VN
  ADC1_CHANNEL_4, //PIN A1.4/R9/32
  ADC1_CHANNEL_5, //PIN A1.5/R8/33
  ADC1_CHANNEL_6, //PIN A1.6/R4/34
  ADC1_CHANNEL_7  //PIN A1.7/R5/35
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
  led_start_blinking();

  //Initialize serial and wait for port to open:
  if (DEBUG) {
    Serial.begin(115200);
    while (!Serial) {
      ; // wait for serial port to connect. Needed for native USB port only
    }
  }

  //setup
  int bootups = setup_deep_sleep();
  setup_adc();
  
  //enter only on reset
  if (bootups == 1) {
    calibrate_adcs();
  }

  setup_display();

  setup_touch();



 
  
  /*
    gpio_set_pull_mode(MODE_SWITCH_PIN, MODE_SWITCH_ON_HIGH ? GPIO_PULLDOWN_ONLY : GPIO_PULLUP_ONLY);
    pinMode(MODE_SWITCH_PIN, MODE_SWITCH_ON_HIGH ? INPUT : INPUT_PULLUP); // MODE SWITCHER INPUT / INPUT_PULLUP for High
    attachInterrupt(digitalPinToInterrupt(MODE_SWITCH_PIN), handleInterrupt, CHANGE);
  */


  bool wifi_mode = false; //digitalRead(MODE_SWITCH_PIN);

  if (wifi_mode) {
    setup_webserver();
    
    if (DEBUG) Serial.println("Starting WiFi Mode");


  } else {

    if (DEBUG) Serial.println("Energy Saving Mode");

    deep_sleep_wake_up_after_time(SLEEP_DURATION_SEC);
    deep_sleep_wake_up_on_touch();

    display_temps_on_display();

    led_stop_blinking();
    deep_sleep_start();
  }
}

/////////////////
// Loop - not executed while deep sleep
/////////////////
void loop() {

  Serial.println("Active...");
  display_temps_on_display();
  //delay(1000);
  //beep(500, 500, BUZZER_PIN);



}





