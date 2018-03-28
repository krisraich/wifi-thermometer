/*
 * Dev board System Led = GPIO_NUM_27
 * PIN 6 = CPU Error
 * https://github.com/espressif/arduino-esp32/blob/master/libraries/ESP32/examples/Timer/RepeatTimer/RepeatTimer.ino
 * LED PWM: https://techtutorialsx.com/2017/06/15/esp32-arduino-led-pwm-fading/ 
 */


bool led_powerd = false;
bool led_blink = false;
BLINK_FREQUENCY current; 


void led_task(void *pvParameter){

  while(true){
    if(led_blink){
      if(led_powerd){
        led_off();
      }else{
         led_on();
      }
    }
    vTaskDelay(current / portTICK_PERIOD_MS);
  }
  
}


void set_blink_frequency(BLINK_FREQUENCY frequency){
  current = frequency;
}

void setup_led(){
  pinMode(ON_BOARD_LED, OUTPUT);
  set_blink_frequency(NORMAL);
  led_off();
  xTaskCreate(&led_task, "led_task", FREE_RTOS_STACK_SIZE, NULL, LED_TASK_PRIORITY, &led_handle);
}

void led_start_blinking(){
  led_blink = true;
  led_on();
}

void led_stop_blinking(){
  led_blink = false;
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


