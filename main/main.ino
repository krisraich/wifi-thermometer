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

#include <driver/adc.h>



/////////////////
// Defines & Constants
/////////////////

#define DEBUG true

#define SLEEP_DURATION_SEC  20       /* Time ESP32 will go to sleep (in seconds) */
#define BUFFER_TIME_EXT_WAKE_UP  500 /* Time ESP32 will wait befor next external wakeup (in milliseconds)*/



/////////////////
// Pin Setup
/////////////////

#define MODE_SWITCH_PIN GPIO_NUM_14 //WIFI SWITCH creats interrupts. With GPIO_NUM_12 connected to high problems!
#define MODE_SWITCH_ON_HIGH true    //true = pulldown, false = pullup

#define DEEP_SLEEP_WAKEUP_SWITCH GPIO_NUM_13 // --> RTC_GPIO04

#define ON_BOARD_LED GPIO_NUM_27
#define ON_BOARD_LED_PULLDOWN_MODE true 

#define ON_BOARD_BUTTON 0
#define ON_BOARD_BUTTON_PULLDOWN_MODE true 

#define ANALOG_PIN_X ADC1_CHANNEL_0
#define ANALOG_PIN_Y ADC1_CHANNEL_3


/////////////////
// Interrupt
/////////////////
void IRAM_ATTR handleInterrupt() {
  detachInterrupt(MODE_SWITCH_PIN);
  if(DEBUG) Serial.println("Mode changed.. restarting");
  delay(100);
  ESP.restart();
}


/////////////////
// Setup
/////////////////
void setup() {

  setup_led();
  led_start_blinking();
  
  //Initialize serial and wait for port to open:
  if(DEBUG){
    Serial.begin(115200);
    while (!Serial) {
      ; // wait for serial port to connect. Needed for native USB port only
    }
  }

  //setup
  int bootups = setup_deep_sleep();
  setup_adc();

  //enter only on reset
  if(bootups == 1){
    calibrate_adcs();
  }
 

/*
  gpio_set_pull_mode(MODE_SWITCH_PIN, MODE_SWITCH_ON_HIGH ? GPIO_PULLDOWN_ONLY : GPIO_PULLUP_ONLY);
  pinMode(MODE_SWITCH_PIN, MODE_SWITCH_ON_HIGH ? INPUT : INPUT_PULLUP); // MODE SWITCHER INPUT / INPUT_PULLUP for High
  attachInterrupt(digitalPinToInterrupt(MODE_SWITCH_PIN), handleInterrupt, CHANGE);
  */
 
 
  
  
  bool wifi_mode = false; //digitalRead(MODE_SWITCH_PIN);
  
  if(wifi_mode){
    
    if(DEBUG) Serial.println("Starting WiFi Mode");

    
  }else{
    
    if(DEBUG) Serial.println("Energy Saving Mode");
  
    deep_sleep_wake_up_after_time(SLEEP_DURATION_SEC);
    deep_sleep_wake_up_on_pin_in(DEEP_SLEEP_WAKEUP_SWITCH, false);

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





