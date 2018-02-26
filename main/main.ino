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
  yyx yxsxsxsxs
  USED REPOSITORIES:
    E-Paper: https://github.com/ZinggJM/GxEPD
    Via Librarie manager: Adafruit GFX, AutoPID


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

  Power Consumption
    Deep Sleep: 1.3mA
    Normal operation: ~ 42mA
    Wifi Server: ~150mA

   Bugs:
    1. Remove Tasks, vTaskDelay
    2. use log not serial print
*/

/////////////////
// Defines & Constants & PINS
/////////////////

#define DEBUG                               true
#define LOG_LOCAL_LEVEL                     ESP_LOG_VERBOSE


#define SLEEP_DURATION_SEC                  30 //Zeitspanne die zwischen den Temperaturen refresh liegen 30 sec = ca 5h history
#define TEMPERATUR_HISTORY_SAMPLE_RATIO     2 // SLEEP_DURATION_SEC * TEMPERATUR_HISTORY_SAMPLE_RATIO * 296 = Time span of history (in sec)

#define WIFI_AP_SSID                        "ESP32"         //Hotspot Wlan SSID
//#define WIFI_AP_PASSWORD                  "TestTest123"  //Hotspot Wlan Passwort, Auskommentieren für offenen Hotspot



//onboard button for Wroom Dev Board
#define ON_BOARD_BUTTON                       0
#define ON_BOARD_BUTTON_PULLDOWN_MODE         true

//onboard LED for LoLin
#define ON_BOARD_LED                          GPIO_NUM_22 //GPIO_NUM_27 for Wroom Dev Board
#define ON_BOARD_LED_PULLDOWN_MODE            true

//Touch button inputs
#define TOUCH_TIME                            3 //time in measuring cycles. 1 Cylce 35ms
#define MODE_TOUCH_BUTTON                     TOUCH_PAD_NUM3 //GPIO 15 See: https://github.com/espressif/arduino-esp32/blob/master/tools/sdk/include/driver/driver/touch_pad.h
#define OK_TOUCH_BUTTON                       TOUCH_PAD_NUM4 // GPIO 13 Button für refresh
#define TOUTCH_THRESHOLD                      0.6 // greater value = more sensitive. Max = 0.99, Min = 0.01

//Pin für LoLin / waveshare 2.9
#define DISPLAY_BUSY                          GPIO_NUM_17 // Display BUSY = any GPIO
#define DISPLAY_RST                           GPIO_NUM_16 //Display RESET = any GPIO
#define DISPLAY_DC                            GPIO_NUM_4 //DATA/COMMAND = any GPIO
#define DISPLAY_CS                            SS //SPI CHIP SELECT = PIN 5
#define DISPLAY_CLK                           SCK //SPI CLOCK = Pin 18
#define DISPLAY_DIN                           MOSI //SPI MOSI (master out slave in) = PIN 23
#define DISABLE_DIAGNOSTIC_OUTPUT             1 // Disables Output of Display class

//thermal control / regulation
#define CHANNEL_1                             GPIO_NUM_14   //Pin out for relais 1
#define CHANNEL_2                             GPIO_NUM_27   //Pin out for relais 2
#define RELAY_PULS_WIDTH                      30 * mS_TO_S_FACTOR //10 sec?
#define REGULATION_CYCLE_TIME                 mS_TO_S_FACTOR //1 sec?

// Battery stuff
//#define IGNORE_BATTERY_VOLTAGE              1 //delete for production
#define BATTERY_VOLTAGE_ANALOG_IN             ADC1_CHANNEL_0 //PIN VP
#define BATTERY_VOLTAGE_DEVIDING_RESISTOR_1   100800
#define BATTERY_VOLTAGE_DEVIDING_RESISTOR_2   100500 //verbunden mit GND und BATTERY_VOLTAGE_ANALOG_IN 
#define MINIMUM_BATTERY_VOLTAGE               2.8       //Abschalt Spannung. für Lithium-Ionen-Akku. Tiefentladung bei 2,5V. Minimale Betriebsspannung ESP32 2,3V + 0,1V Dropout vom Regler = 2,4V . Bei 2,8V sollt genügend restkapazität vorhanden sein */
#define MAX_BATTERY_VOLTAGE                   4.2   //Maximale LiIon Zellenspannung
#define SWITCH_TO_POWERSAVE_WHEN_BAT_LOW      7   //Schalte unter X prozent Batterie in POWER_SAVE modus. Auskommentieren um zu deaktivieren 


//times and numbers
#define uS_TO_S_FACTOR                        1000000     // Conversion factor for micro seconds to seconds 
#define mS_TO_S_FACTOR                        1000        // Conversion factor for milli seconds to seconds 

#define MIN_TIME_BETWEEN_REFRESH              1000    // zeit die vergehen muss bevor der benutzer die temperaturen aktualisieren kann (ms)
#define SHOW_MENU_ON_DISPLAY_TIME             15      // Wie lange soll das Menü auf dem Display angezeigt werden (sec)

//logs
#define LOG_TAG_MAIN                          "Main"
#define LOG_TAG_ADC                           "ADC"
#define LOG_TAG_BLUETOOTH                     "Bluetooth"
#define LOG_TAG_DATA_STORE                    "Data Store"
#define LOG_TAG_DEEP_SLEEP                    "Deep Sleep"
#define LOG_TAG_DISPLAY                       "Display"
#define LOG_TAG_LED                           "Led"
#define LOG_TAG_LOG                           "Logger"
#define LOG_TAG_RECORDER                      "Recorder"
#define LOG_TAG_REGULATION                    "Regulation"
#define LOG_TAG_TOUCH                         "Touch"
#define LOG_TAG_WEBSERVER                     "Webserver"


/////////////////
// LIBS
/////////////////

//log
#include "esp_log.h"

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
#include "res/img_logo.h"
#include "res/img_icons.h"
#include "res/img_battery_low.h"

//webserver
#include <WiFi.h>
#include <FS.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "res/bin_favicon.h"

//EEPROM
#include "EEPROM.h"

//regulation
#include <AutoPID.h>

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

const gpio_num_t TEMP_CONTROL_PINS[] {
  CHANNEL_1,
  CHANNEL_2
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

bool menu_open = false; //menu is on display
unsigned long last_refresh = 0; //last display refresh (controll refreshrate)
unsigned long last_interaction_since = 0; //time when menu was opend for auto closing

OPERATION_MODE current_operation_mode;
OPERATION_MODE selected_operation_mode;


hw_timer_t * led_timer = NULL;
hw_timer_t * regulation_timer = NULL;
hw_timer_t * refresh_timer = NULL;
hw_timer_t * touch_timer = NULL;

/////////////////
// Init Display
/////////////////

GxIO_Class io(SPI, DISPLAY_CS, DISPLAY_DC, DISPLAY_RST);  //SPI,SS,DC,RST
GxGDEH029A1 display(io, DISPLAY_RST, DISPLAY_BUSY);  //io,RST,BUSY

/////////////////
// maintenances tasks
/////////////////

void refresh_display(void *pvParameter) {
  while (true) {

    check_battery_life();

    //only refresh when user hasn't refreshed it
    if (!menu_open && ((millis() - last_refresh) > (SLEEP_DURATION_SEC * mS_TO_S_FACTOR))) {
      if (DEBUG) Serial.println("Auto refreshing temps: ");
      update_display();
    }

    record_temperatures();
    vTaskDelay(SLEEP_DURATION_SEC * mS_TO_S_FACTOR / portTICK_PERIOD_MS);
  }
}

void auto_close_menu(void *pvParameter) {
  while (true) {
    if (menu_open && ((millis() - last_interaction_since) > (SHOW_MENU_ON_DISPLAY_TIME * mS_TO_S_FACTOR))) {
      //auto close menu
      if (DEBUG) Serial.println("Auto closing menu");
      menu_open = false;
      update_display();
      if (current_operation_mode == POWER_SAVING || current_operation_mode == BT_LE_SLAVE) {
        start_power_saving_mode();
      }
    }
    vTaskDelay(SHOW_MENU_ON_DISPLAY_TIME * 50 / portTICK_PERIOD_MS); //50 for no haste...
  }
}


/////////////////
// Setup
/////////////////
void setup() {

  /*
   * Init hardware, which is used every time
   */
   
  setup_led();

  setup_log();

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

  check_battery_life();

  //enter only on reset
  if (bootups == 1) {
    calibrate_adcs();
  }

  //must set befor touch
  current_operation_mode = get_last_operation_mode();
  selected_operation_mode = get_last_operation_mode(); //using current_operation_mode would copy the reference...

  //also needed for deepsleep
  setup_touch();


  setup_recorder();

//  if (DEBUG) Serial.println("Starting operation mode: " + String(operation_mode_to_string(current_operation_mode)));
  ESP_LOGI(LOG_TAG_MAIN, "Starting '%s' mode", operation_mode_to_string(current_operation_mode));


  //todo: check if menu is open


  /*
   * Now check which mode to load,
   * If WiFi mode, load more stuff
   */
  switch (current_operation_mode) {
    default:
      ESP_LOGW(LOG_TAG_MAIN, "Unknown operation mode. Fall back to POWER_SAVE");
      //set default
      save_operation_mode(POWER_SAVING);
    case BT_LE_SLAVE:
      send_data_with_ble();
    case POWER_SAVING:
      if (!menu_open) {
        //after ini, show temps. except when menu is open
        start_power_saving_mode();
      }
      return;
    case WIFI_SERVER:
      start_wifi_mode();
  }
}

void check_battery_life() {
#if ! defined(IGNORE_BATTERY_VOLTAGE)

#if defined(SWITCH_TO_POWERSAVE_WHEN_BAT_LOW)
  if (current_operation_mode == WIFI_SERVER && get_battery_percente() <= SWITCH_TO_POWERSAVE_WHEN_BAT_LOW) {
    if (DEBUG) Serial.println("Battery is nearly empty.. switchng to POWER_SAVE");
    save_operation_mode(POWER_SAVING);
    ESP.restart();
  }
#endif

  if (get_battery_voltage() <= MINIMUM_BATTERY_VOLTAGE) {
    if (DEBUG) Serial.println("Battery is empty... shutting down");
    show_empty_battery();
    shutdown_esp();
  }
#endif
}

void start_wifi_mode() {
  update_display();
  setup_webserver();
  setup_regulation();
  //xTaskCreate(&refresh_display, "refresh_display", FREE_RTOS_STACK_SIZE, NULL, REFRESH_TASK_PRIORITY, &refresh_handle);
}

void send_data_with_ble(){
  
}

void start_power_saving_mode() {

  if (current_operation_mode == BT_LE_SLAVE) {
    //search for wifi devices and send data via BLE
    if (DEBUG) {
      Serial.println("Searching Devices in WiFi Mode");
      setup_bluetooth();
      delay(200);
      Serial.println("Sending Data via Bluetooth Low Energy (BLE)");
    }

  }

  update_display();

  deep_sleep_wake_up_after_time(SLEEP_DURATION_SEC);
  deep_sleep_wake_up_on_touch();
  led_stop_blinking();
  deep_sleep_start();
}

void default_procedure_on_error() {
  if (DEBUG) Serial.println("Default procedure startet.. rebooting into POWER_SAVING");
  save_operation_mode(POWER_SAVING);
  prepare_to_shutdown();
  ESP.restart();
}


void prepare_to_shutdown() {
  if (current_operation_mode == WIFI_SERVER) {
    stop_webserver();
    stop_regulation();
  }
  stop_touch();
}


void touch_button_pressed(touch_pad_t pressed_button, bool on_boot) {

  if (DEBUG) Serial.println("Touch input: " + String(pressed_button == OK_TOUCH_BUTTON ? "ok/refresh" : "mode"));

  if (on_boot && pressed_button == OK_TOUCH_BUTTON) {
    //esp was woken up by user in power saving mode.,
    //so do nothing and just refresh the temps
  } else if (pressed_button == MODE_TOUCH_BUTTON) {

    last_interaction_since = millis();

    //when esp was woken up by user pressing the mode button,
    //always show menu, becaus it'll never go to sleep when the menu is open
    //since the menu is also called when esp is awake, just ignor boot option and show menu
    if (menu_open) {
      if (DEBUG) Serial.print("switch mode from " + String(operation_mode_to_string(selected_operation_mode)));
      selected_operation_mode = cycle_through_modes(selected_operation_mode);
      if (DEBUG) Serial.println(" to " + String(operation_mode_to_string(selected_operation_mode)));
    } else {
      menu_open = true;
      //if (menu_close_handle == NULL) {
        //xTaskCreate(&auto_close_menu, "auto_close_menu", FREE_RTOS_STACK_SIZE, NULL, AUTO_CLOSE_TASK_PRIORITY, &menu_close_handle);
      //}
    }
    show_menu(selected_operation_mode);

  } else if (pressed_button == OK_TOUCH_BUTTON) {
    // ok button is refresh button when menu is active
    if (menu_open) {
      //select menu ....
      if (DEBUG) Serial.println("select mode! Going from " + String(operation_mode_to_string(current_operation_mode)) +
                                  " to " + String(operation_mode_to_string(selected_operation_mode)));
      menu_open = false;

      switch (selected_operation_mode) {

        //only when mode was active and user waked up esp
        case POWER_SAVING:
          save_operation_mode(POWER_SAVING);
          switch (current_operation_mode) {

            //going from BT_LE_SLAVE to POWER_SAVING
            case BT_LE_SLAVE:
            //going from POWER_SAVING to POWER_SAVING
            case POWER_SAVING:
              current_operation_mode = POWER_SAVING;
              start_power_saving_mode(); //menu is closed.. now go to sleep
              break;

            //going from WIFI_SERVER to POWER_SAVING
            case WIFI_SERVER:
              prepare_to_shutdown();
              ESP.restart(); //needs restart to turn off wifi
              break;

            //default: mode has been saved and it should load next boot
            default: ESP.restart();
          }
          break;

        //only when mode was active and user waked up esp
        case BT_LE_SLAVE:
          save_operation_mode(BT_LE_SLAVE);
          switch (current_operation_mode) {

            //going from BT_LE_SLAVE to BT_LE_SLAVE
            case BT_LE_SLAVE:
            //going from POWER_SAVING to BT_LE_SLAVE
            case POWER_SAVING:
              current_operation_mode = BT_LE_SLAVE;
              start_power_saving_mode(); //menu is closed.. now go to sleep
              break;

            //going from WIFI_SERVER to POWER_SAVING
            case WIFI_SERVER:
              prepare_to_shutdown();
              ESP.restart(); //needs restart to turn off wifi
              break;

            //default: mode has been saved and it should load next boot
            default: ESP.restart();
          }
          break;

        case WIFI_SERVER:

          //don't switch to wifi mode when battery is weak
#if defined(SWITCH_TO_POWERSAVE_WHEN_BAT_LOW)
          if (get_battery_percente() <= SWITCH_TO_POWERSAVE_WHEN_BAT_LOW) {
            if (DEBUG) Serial.println("Battery is nearly empty.. cant switch to wifi mode");
            show_empty_battery();
            vTaskDelay(2000 / portTICK_PERIOD_MS); //show for 2 seconds
            break;
          }
#endif

          save_operation_mode(WIFI_SERVER);
          switch (current_operation_mode) {

            //going from BT_LE_SLAVE to WIFI_SERVER
            case BT_LE_SLAVE:
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
            default: default_procedure_on_error();
          }
          break;

        case SHUTDOWN:
          //save_operation_mode(POWER_SAVING);
          show_shutdown();
          prepare_to_shutdown();
          shutdown_esp();
          break;

        //default fallback
        default: default_procedure_on_error();
      }

      //if nothing todo.. update display
      update_display();

    } else if (millis() - last_refresh > MIN_TIME_BETWEEN_REFRESH) {
      if (DEBUG) Serial.println("user refresh temps: ");
      update_display();
    }

  } else {
    if (DEBUG) Serial.println("You shouldn't see this...");
  }

}

/////////////////
// Loop - not executed in power saving mode
/////////////////
void loop() {
  delay(100000);
}

