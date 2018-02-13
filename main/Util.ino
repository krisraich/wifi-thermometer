
#define DETECTION_PULLING_RES 10 //pull every 10ms

bool touch_has_been_active(int milli_seconds, uint8_t pin, uint16_t threshold){
  do{
    delay(DETECTION_PULLING_RES);
    milli_seconds -= DETECTION_PULLING_RES;
  }while((touchRead(pin) > threshold) && (milli_seconds > 0));
  return milli_seconds == 0;
}

bool button_has_been_pressed(int milli_seconds, uint8_t pin, bool pull_down){
  pinMode(pin, pull_down ? INPUT_PULLUP : INPUT);
  do{
    delay(DETECTION_PULLING_RES);
    milli_seconds -= DETECTION_PULLING_RES;
  }while((digitalRead(pin) == pull_down ? 0 : 1) && milli_seconds > 0);
  return milli_seconds == 0;
}



