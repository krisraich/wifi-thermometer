/*
 * Dev board System Led = GPIO_NUM_27
 * PIN 6 = CPU Error
 * https://github.com/espressif/arduino-esp32/blob/master/libraries/ESP32/examples/Timer/RepeatTimer/RepeatTimer.ino
 * LED PWM: https://techtutorialsx.com/2017/06/15/esp32-arduino-led-pwm-fading/
 */


bool led_powerd = false;
hw_timer_t * timer = NULL;

void IRAM_ATTR toogle_led(){
  if(led_powerd)
    led_off();
  else
    led_on(); 
}

void set_blink_frequency(BLINK_FREQUENCY frequency){
  timerAlarmWrite(timer, uS_TO_S_FACTOR / frequency , true);
}

void setup_led(){
  pinMode(ON_BOARD_LED, OUTPUT);
  led_on();
  timer = timerBegin(0, 80, true); //timer id 0 bis 3, 80 = devider
  timerAttachInterrupt(timer, &toogle_led, true);
  set_blink_frequency(NORMAL);
  
}

void led_start_blinking(){
  if(DEBUG) Serial.println("Start LED blinking");
  led_on();
  timerAlarmEnable(timer);
}

void led_stop_blinking(){
  if(DEBUG) Serial.println("Stop LED blinking");
  timerEnd(timer);
  led_off();
}

void led_on(){
  led_powerd = true;
  digitalWrite(ON_BOARD_LED, ON_BOARD_LED_PULLDOWN_MODE ? LOW : HIGH);
}
void led_off(){
  led_powerd = false;
  digitalWrite(ON_BOARD_LED, ON_BOARD_LED_PULLDOWN_MODE ? HIGH : LOW);
}


