
#define DETECTION_PULLING_RES 10 //pull every 10ms

static bool touch_has_been_active(int milli_seconds, uint8_t pin, uint16_t threshold){
  do{
    delay(DETECTION_PULLING_RES);
    milli_seconds -= DETECTION_PULLING_RES;
  }while((touchRead(pin) > threshold) && (milli_seconds > 0));
  return milli_seconds == 0;
}

static bool button_has_been_pressed(int milli_seconds, uint8_t pin, bool pull_down){
  pinMode(pin, pull_down ? INPUT_PULLUP : INPUT);
  do{
    delay(DETECTION_PULLING_RES);
    milli_seconds -= DETECTION_PULLING_RES;
  }while((digitalRead(pin) == pull_down ? 0 : 1) && milli_seconds > 0);
  return milli_seconds == 0;
}


static inline const char* operation_mode_to_string(OPERATION_MODE operation_mode){
  switch (operation_mode){
    case POWER_SAVING: return "POWER_SAVING";
    case WIFI_SERVER:  return "WIFI_SERVER";
    case BT_LE_SLAVE:  return "BT_LE_SLAVE";
    case SHUTDOWN:     return "SHUTDOWN";
    default:           return "[Unknown OPERATION_MODE]";
  }
}

static inline String print_regression_parameter(REGRESSION_PARAMETER in){
  return "{a: " + String(in.param_a) + ", b: " + String(in.param_b) + ", c: " + String(in.param_c) + ", d: " + String(in.param_d) + " }";
}

static int count_adc_channels(){
  return (sizeof(ADC_CHANNELS)/sizeof(*ADC_CHANNELS));
}

static OPERATION_MODE cycle_through_modes(OPERATION_MODE operation_mode){
   switch (operation_mode){
    case POWER_SAVING: return WIFI_SERVER;
    case WIFI_SERVER:  return BT_LE_SLAVE;
    case BT_LE_SLAVE:  return SHUTDOWN;
    default:
    case SHUTDOWN:     return POWER_SAVING;
  }
}



