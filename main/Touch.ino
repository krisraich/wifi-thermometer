/*
 * http://esp-idf.readthedocs.io/en/latest/api-reference/peripherals/touch_pad.html
 * https://github.com/espressif/arduino-esp32/blob/master/libraries/ESP32/examples/Touch/TouchInterrupt/TouchInterrupt.ino
 * https://github.com/espressif/esp-idf/blob/master/components/driver/include/driver/touch_pad.h
 */

 void setup_touch(){

  touch_pad_set_trigger_mode(TOUCH_TRIGGER_ABOVE);


  //touchAttachInterrupt(MODE_TOUCH_BUTTON, mode_button_pressed, TOUCH_THRESHOLD);
  //touchAttachInterrupt(OK_TOUCH_BUTTON, ok_button_pressed, TOUCH_THRESHOLD);
 }

