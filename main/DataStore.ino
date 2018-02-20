/*
 * https://github.com/espressif/arduino-esp32/blob/master/libraries/EEPROM/examples/eeprom_write/eeprom_write.ino
 * https://github.com/espressif/arduino-esp32/blob/master/libraries/EEPROM/EEPROM.h
 */

#define EEPROM_SIZE 64

#define LAST_OPERATION_MODE_ADDRESS 0

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

