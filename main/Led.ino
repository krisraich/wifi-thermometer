/*
 * Dev board System Led = GPIO_NUM_27
 * PIN 6 = CPU Error
 * https://github.com/espressif/arduino-esp32/blob/master/libraries/ESP32/examples/Timer/RepeatTimer/RepeatTimer.ino
 */

//change this
const int BLINK_FREQ = 10;

bool led_powerd = false;
hw_timer_t * timer = NULL;

void IRAM_ATTR toogle_led(){
  if(led_powerd)
    led_off();
  else
    led_on(); 
}

void setup_led(){
  pinMode(ON_BOARD_LED, OUTPUT);
  timer = timerBegin(0, 80, true); //timer id 0 bis 3, 80 = devider
  timerAttachInterrupt(timer, &toogle_led, true);
  timerAlarmWrite(timer, 1000000 / BLINK_FREQ , true);
  
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


