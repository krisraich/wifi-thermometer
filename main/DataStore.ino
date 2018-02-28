/*
 * https://github.com/espressif/arduino-esp32/blob/master/libraries/EEPROM/examples/eeprom_write/eeprom_write.ino
 * https://github.com/espressif/arduino-esp32/blob/master/libraries/EEPROM/EEPROM.h
 */ 

#define EEPROM_SIZE 64

#define LAST_OPERATION_MODE_ADDRESS 0

//store float
#define CAL_A_ADDRESS 10
#define CAL_B_ADDRESS 14
#define CAL_C_ADDRESS 18
#define CAL_D_ADDRESS 24

void setup_data_store(){
  EEPROM.begin(EEPROM_SIZE);
  if (DEBUG) Serial.println("Init EEPROM"); 
}


OPERATION_MODE get_last_operation_mode(){
  uint8_t value = EEPROM.read(LAST_OPERATION_MODE_ADDRESS);
  return static_cast<OPERATION_MODE>(value);
}

void save_operation_mode(OPERATION_MODE operation_mode){
  OPERATION_MODE saved_operation_mode = get_last_operation_mode();
  
  //minimize writes
  if(saved_operation_mode != operation_mode){
    uint8_t value = static_cast<uint8_t>(operation_mode);
    EEPROM.write(LAST_OPERATION_MODE_ADDRESS, value);
    EEPROM.commit();
  }
}

REGRESSION_PARAMETER read_regression_params(){
  REGRESSION_PARAMETER temp = {};

  temp.param_a = get_float_from_address(CAL_A_ADDRESS);
  temp.param_b = get_float_from_address(CAL_B_ADDRESS);
  temp.param_c = get_float_from_address(CAL_C_ADDRESS);
  temp.param_d = get_float_from_address(CAL_D_ADDRESS);

  return temp;
}


void save_regression_params(REGRESSION_PARAMETER params){
  store_float_at_address(CAL_A_ADDRESS, params.param_a);
  store_float_at_address(CAL_B_ADDRESS, params.param_b);
  store_float_at_address(CAL_C_ADDRESS, params.param_c);
  store_float_at_address(CAL_D_ADDRESS, params.param_d);
  if (DEBUG) Serial.println("Regression values saved"); 
  EEPROM.commit();
}

float get_float_from_address(int address){
  uint8_t buffer_array[4];
  for(int i = 0; i < 4; i++){
    buffer_array[i] = EEPROM.read(address + i);
  }
  return *(float *)&buffer_array;
}

void store_float_at_address(int address, float value){
  uint8_t buffer_array[4];
  memcpy(buffer_array, (uint8_t*) (&value), 4);
  for(int i = 0; i < 4; i++){
    EEPROM.write(address + i, buffer_array[i]);
  }
}

