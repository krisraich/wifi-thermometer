/*
 * http://esp-idf.readthedocs.io/en/latest/api-reference/peripherals/touch_pad.html
 * https://github.com/espressif/arduino-esp32/blob/master/libraries/ESP32/examples/Touch/TouchInterrupt/TouchInterrupt.ino
 * https://github.com/espressif/esp-idf/blob/master/components/driver/include/driver/touch_pad.h
 * 
 * https://github.com/espressif/esp-idf/blob/c3bec5b103888217d53527924e5185d49f9636ea/examples/peripherals/touch_pad_interrupt/main/tp_interrupt_main.c
 * https://github.com/espressif/esp-idf/blob/master/components/driver/rtc_module.c
 * https://techtutorialsx.com/2017/09/30/esp32-arduino-external-interrupts/
 */

#define DECAYING_FAKTOR 3 //lower = faster degreading

int s_pad_activated[TOUCH_PAD_MAX];
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

/*
  Read values sensed at all available touch pads.
  Use 2 / 3 of read value as the threshold
  to trigger interrupt when the pad is touched.
  Note: this routine demonstrates a simple way
  to configure activation threshold for the touch pads.
  Do not touch any pads when this routine
  is running (on application start).
 */
void tp_set_thresholds(void){
  uint16_t touch_value;
  //delay some time in order to make the filter work and get a initial value
  vTaskDelay(500/portTICK_PERIOD_MS);

  for (touch_pad_t current_touch : TOUCH_BUTTONS){  
    //read filtered value
    touch_pad_read_filtered(current_touch, &touch_value);
    //set interrupt threshold.
    ESP_ERROR_CHECK(touch_pad_set_thresh(current_touch, touch_value * 2 / 3));
  }
}

/*
 * 
 * Use other filter mode for touch detection through material 
 * https://github.com/espressif/esp-idf/blob/c3bec5b103888217d53527924e5185d49f9636ea/examples/peripherals/touch_pad_interrupt/main/tp_interrupt_main.c
 * 
  Check if any of touch pads has been activated
  by reading a table updated by rtc_intr()
  If so, then print it out on a serial monitor.
  Clear related entry in the table afterwards
  In interrupt mode, the table is updated in touch ISR.
  In filter mode, we will compare the current filtered value with the initial one.
  If the current filtered value is less than 99% of the initial value, we can
  regard it as a 'touched' event.
  When calling touch_pad_init, a timer will be started to run the filter.
  This mode is designed for the situation that the pad is covered
  by a 2-or-3-mm-thick medium, usually glass or plastic.
  The difference caused by a 'touch' action could be very small, but we can still use
  filter mode to detect a 'touch' event.
 */
void tp_read_task(void *pvParameter){
  
  //call when touch waked up ESP32
  if(was_waked_up_by_touch()){
    touch_button_pressed(get_wakeup_toch(), true);
  }
  
  while (true) {
    for (touch_pad_t current_touch : TOUCH_BUTTONS){
      if (s_pad_activated[current_touch] >= TOUCH_TIME * DECAYING_FAKTOR) {

        touch_button_pressed(current_touch, false);
        
        // Clear information on pad activation
        portENTER_CRITICAL(&mux);
        s_pad_activated[current_touch] = 0;
        portEXIT_CRITICAL(&mux);
      }else if(s_pad_activated[current_touch] > 0){
        portENTER_CRITICAL_ISR(&mux);
        //decaying valuee
        s_pad_activated[current_touch]--;
        portEXIT_CRITICAL_ISR(&mux);
      }
    }
    vTaskDelay(25);
  }
}

/*
  Handle an interrupt triggered when a pad is touched.
  Recognize what pad has been touched and save it in a table.
 */
void IRAM_ATTR tp_rtc_intr(void * arg){
  uint32_t pad_intr = touch_pad_get_status();
  
  //clear interrupt
  touch_pad_clear_status();
  
  for (touch_pad_t current_touch : TOUCH_BUTTONS){
    if ((pad_intr >> current_touch) & 0x01) {
      portENTER_CRITICAL_ISR(&mux);
      s_pad_activated[current_touch] += DECAYING_FAKTOR;
      portEXIT_CRITICAL_ISR(&mux);
    }
  }
}

/*
 * Before reading touch pad, we need to initialize the RTC IO.
 */
void tp_touch_pad_init(){
  for (touch_pad_t current_touch : TOUCH_BUTTONS){ 
    //init RTC IO and mode for touch pad.
    touch_pad_config(current_touch, 0); //Enabling sensor but with threshold 0
  }
}

void setup_touch(){

  // Initialize touch pad peripheral, it will start a timer to run a filter
  if (DEBUG) Serial.println("Initializing touch pad");
  
  touch_pad_init();

  // Initialize and start a software filter to detect slight change of capacitance.
  touch_pad_filter_start(10);
  
  // Set measuring time and sleep time
  // In this case, measurement will sustain 0xffff / 8Mhz = 8.19ms
  // Meanwhile, sleep time between two measurement will be 0x1000 / 150Khz = 27.3 ms
  touch_pad_set_meas_time(0x1000, 0xffff);

  //set reference voltage for charging/discharging
  // In this case, the high reference valtage will be 2.4V - 1.5V = 0.9V
  // The low reference voltage will be 0.8V, so that the procedure of charging
  // and discharging would be very fast.
  touch_pad_set_voltage(TOUCH_HVOLT_2V4, TOUCH_LVOLT_0V8, TOUCH_HVOLT_ATTEN_1V5);
  // Init touch pad IO
  tp_touch_pad_init();
  // Set thresh hold
  tp_set_thresholds();
  // Register touch interrupt ISR
  touch_pad_isr_register(tp_rtc_intr, NULL);

  // Start a task to show what pads have been touched
  xTaskCreate(&tp_read_task, "touch_pad_read_task", 2048, NULL, 5, NULL);

  //enable interrupts
  touch_pad_intr_enable();  //touch_pad_intr_disable()

}
