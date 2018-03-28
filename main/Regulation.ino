/*
 * Temperture regulation wioth PID
 * https://github.com/r-downing/AutoPID
 * https://github.com/r-downing/AutoPID/blob/master/AutoPID.cpp
 * http://ryandowning.net/AutoPID/#pwm-relay-control
 * https://en.wikipedia.org/wiki/PID_controller
 * 
 * Must be tested
 */

//pid settings and gains
#define BANGBANG 6 //if temperature is more than X degrees below or above setpoint, OUTPUT will be set to min or max respectively
#define KP .12
#define KI .0003
#define KD 0


//Define Variables we'll be connecting to
double setpoint_1, input_1, setpoint_2, input_2;
bool relay_state1 = false, relay_state2 = false;

//current input channels
adc1_channel_t *channel_1, *channel_2;

AutoPIDRelay pid_1(&input_1, &setpoint_1, &relay_state1, RELAY_PULS_WIDTH, KP, KI, KD);
AutoPIDRelay pid_2(&input_2, &setpoint_2, &relay_state2, RELAY_PULS_WIDTH, KP, KI, KD);

void regulation_task(void *pvParameter) {
  while(true){
    if(!pid_1.isStopped()){
      set_temp_channel1();
      pid_1.run();
      digitalWrite(CHANNEL_1, relay_state1);
    }
    
    if(!pid_2.isStopped()){
      set_temp_channel1();
      pid_2.run();
      digitalWrite(CHANNEL_2, relay_state2);
    }

    vTaskDelay(REGULATION_CYCLE_TIME / portTICK_PERIOD_MS);
  }

}

void setup_regulation(){
  if (DEBUG) Serial.println("Init Regulation"); 

  pinMode(CHANNEL_1, OUTPUT);
  pinMode(CHANNEL_2, OUTPUT);

  digitalWrite(CHANNEL_1, LOW);
  digitalWrite(CHANNEL_2, LOW);

  pid_1.setBangBang(BANGBANG);
  pid_2.setBangBang(BANGBANG);

  pid_1.setTimeStep(REGULATION_CYCLE_TIME); //1000ms is default
  pid_2.setTimeStep(REGULATION_CYCLE_TIME); //1000ms is default
  
  clear_channel1();
  clear_channel2();
  
  
  xTaskCreate(&regulation_task, "regulation_task", FREE_RTOS_STACK_SIZE, NULL, REGULATION_TASK_PRIORITY, &regulation_handle);
}

void setup_channel1(adc1_channel_t input, float target){
  //stopping & resetting
  clear_channel1();
  //setting channel
  channel_1 = &input;
  //setting temps befor running regulation
  set_temp_channel1();
  //setting target temps
  set_target_temperature_channel1(target);
  //execute
  pid_1.run();
}

void setup_channel2(adc1_channel_t input, float target){
  clear_channel2();
  channel_2 = &input;
  set_temp_channel2();
  set_target_temperature_channel2(target);
  pid_2.run();
}

void clear_channel1(){
  pid_1.stop();
}

void clear_channel2(){
  pid_2.stop();
}

void set_target_temperature_channel1(float target){
  setpoint_1 = target;
}
void set_target_temperature_channel2(float target){
  setpoint_2 = target;
}

void set_temp_channel1(){
  input_1 = get_temperature_from_channel(*channel_1);
}
void set_temp_channel2(){
  input_2 = get_temperature_from_channel(*channel_2);
}

bool regulation_is_running_on_channel(adc1_channel_t channel){
  if( &channel != channel_1 && &channel != channel_2) return false;
  
  if(&channel == channel_1 && !pid_1.isStopped()) return true;
  if(&channel == channel_2 && !pid_2.isStopped()) return true;

  return false;
}


