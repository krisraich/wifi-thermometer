/*
 * Deep Sleep stuff
 * https://github.com/espressif/arduino-esp32/blob/master/libraries/ESP32/examples/DeepSleep/TimerWakeUp/TimerWakeUp.ino
 * https://esp-idf.readthedocs.io/en/v2.0/api/system/deep_sleep.html
 * https://github.com/SensorsIot/ESP32-Deep-Sleep
 * https://esp-idf.readthedocs.io/en/v3.0-rc1/api-reference/system/sleep_modes.html?highlight=deep%20sleep#power-down-of-rtc-peripherals-and-memories
 * 
 * !Only pins that support both input & output have integrated pull-up and pull-down resistors. Input-only GPIOs 34-39 do not.
 */


RTC_DATA_ATTR bool ext_wakeup = false;
RTC_DATA_ATTR int boot_count = 0;

int wakeup_reason;
touch_pad_t wakeup_toch;

int get_bootups(){
  return boot_count;
}

bool was_waked_up_by_touch(){
  return wakeup_reason == 4;
}

touch_pad_t get_wakeup_toch(){
  return wakeup_toch;
}

int setup_deep_sleep(){
  boot_count++;

  wakeup_reason = esp_sleep_get_wakeup_cause();
  ext_wakeup = (wakeup_reason == 1 ||  //ESP_DEEP_SLEEP_WAKEUP_EXT0
                wakeup_reason == 2 || //ESP_DEEP_SLEEP_WAKEUP_EXT1
                wakeup_reason == 4); //ESP_DEEP_SLEEP_WAKEUP_TOUCHPAD

  if(was_waked_up_by_touch()){
    wakeup_toch = esp_sleep_get_touchpad_wakeup_status();
  }
                
  if(DEBUG){
    Serial.println("Wake up No. " + String(boot_count) + " by " + (ext_wakeup ? "touch" : "timer"));
    //print_wakeup_reason();
  }
  return boot_count;
}


void deep_sleep_wake_up_after_time(int sleepSeconds){  
  esp_sleep_enable_timer_wakeup(sleepSeconds * uS_TO_S_FACTOR);
  if(DEBUG){
    Serial.println("Setup ESP32 to wake up after " + String(sleepSeconds) +  " Seconds");
  }
}

//needs touch interrupt to work...
void deep_sleep_wake_up_on_touch(){ 
  if(DEBUG) Serial.println("Setup ESP32 to wake up on toch");
  esp_sleep_enable_touchpad_wakeup();
}

void deep_sleep_wake_up_on_pin_in(gpio_num_t rtc_gpio, bool on_high){ 

  if(DEBUG && rtc_gpio >= 34){
    Serial.println("PIN DOES NOT SUPPORT PULL UP/DOWN --> Needs external resistor!");
  }

  //flankengetriggert
  gpio_set_intr_type(rtc_gpio, on_high ? GPIO_INTR_POSEDGE : GPIO_INTR_NEGEDGE);

  gpio_set_pull_mode(rtc_gpio, on_high ? GPIO_PULLDOWN_ONLY : GPIO_PULLUP_ONLY);

  esp_sleep_enable_ext0_wakeup(rtc_gpio, on_high ? 1 : 0); //1 = High, 0 = Low
  if(DEBUG){
    Serial.println("Setup ESP32 to wake up when GPIO_" + String(rtc_gpio) +  " is " + (on_high ? "high" : "low"));
  }
}

void deep_sleep_start(){
  if(DEBUG) Serial.println("Going to sleep now");
  
  //little timeout 
  delay(1000);
  
  esp_deep_sleep_start();
}

void shutdown_esp(){
  if(DEBUG) Serial.println("Shutting down now");
  
  //little timeout 
  delay(100);
  
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);
  deep_sleep_wake_up_after_time(2147483647); //68 years. should be enough
  esp_deep_sleep_start();
}

/*
Method to print the reason by which ESP32
has been awaken from sleep
*/
void print_wakeup_reason(){
  switch(wakeup_reason)
  {
    case 1  : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case 2  : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case 3  : Serial.println("Wakeup caused by timer"); break;
    case 4  : Serial.println("Wakeup caused by touchpad on pin: " + String(wakeup_toch)); break;
    case 5  : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.println("Wakeup was not caused by deep sleep"); break;
  }
}
