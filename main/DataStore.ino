/*
 * https://github.com/espressif/arduino-esp32/blob/master/libraries/EEPROM/examples/eeprom_write/eeprom_write.ino
 * https://github.com/espressif/arduino-esp32/blob/master/libraries/EEPROM/EEPROM.h
 */ 

#define EEPROM_SIZE 512

#define LAST_OPERATION_MODE_ADDRESS 0


#define WIFI_SSID_ADDRESS 9 //WiFi SSID Max len = 32 chars
//wifi SSID name Address 10 to 42

#define WIFI_PWD_ADDRESS 43  //WiFi pwd max len = 62 A pass-phrase is a sequence of between 8 and 63 ASCII-encoded characters
//wifi Passwd Address from 44 to 107

//16 Bytes per param set * 6 = 104 Bytes -> EEPROM_SIZE mind. 304
#define REGRESSION_PARAMS_START 200 




void setup_data_store(){
  EEPROM.begin(EEPROM_SIZE);
  if (DEBUG) Serial.println("Init EEPROM"); 
}

void clear_eeprom(){
  for(int i = 0; i < EEPROM_SIZE; i++){
    EEPROM.write(i, 0);
  }
  EEPROM.commit();
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

REGRESSION_PARAMETER read_regression_params(int index){
  //read param, and store in eeprom
  REGRESSION_PARAMETER tmp = {
    get_float_from_address(REGRESSION_PARAMS_START + (index * 16)),
    get_float_from_address(REGRESSION_PARAMS_START + (index * 16) + 4),
    get_float_from_address(REGRESSION_PARAMS_START + (index * 16) + 8),
    get_float_from_address(REGRESSION_PARAMS_START + (index * 16) + 12),
  };

  return tmp;
}
REGRESSION_PARAMETER read_regression_params_for_battery(){
  return read_regression_params(count_adc_channels()); //append after temp regression
}


void save_regression_params(REGRESSION_PARAMETER params, int index){
  store_float_at_address(REGRESSION_PARAMS_START + (index * 16), params.param_a);
  store_float_at_address(REGRESSION_PARAMS_START + (index * 16) + 4, params.param_b);
  store_float_at_address(REGRESSION_PARAMS_START + (index * 16) + 8, params.param_c);
  store_float_at_address(REGRESSION_PARAMS_START + (index * 16) + 12, params.param_d);
  if (DEBUG) Serial.println("Regression values saved"); 
  EEPROM.commit();
}
void save_regression_params_for_battery(REGRESSION_PARAMETER params){
  save_regression_params(params, count_adc_channels());
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


void write_string_to_address(int address, String data){
  uint8_t len = data.length();

  EEPROM.write(address, len);

  char buf[63]; //max wifi chars..
  int start_address = address + 1;

  data.toCharArray(buf, len+1); //+1 is needed apparently..
  
  for(int i = 0; i < len; i++){
    EEPROM.write(start_address + i, buf[i]);
  }
  EEPROM.commit();
}

void delete_string_from_address(int address){
  EEPROM.write(address, 0);
  EEPROM.commit();
}

String read_string_from_address(int address){
  String buf = "";
  int start_address = address + 1;
  int stop_address = start_address + read_string_leng_from_address(address);
  
  for(int i = start_address; i < stop_address; i++){
    buf += ((char) EEPROM.read(i));
  }
  return buf;
}

uint8_t read_string_leng_from_address(int address){
  return EEPROM.read(address);
}

bool has_string_on_address(int address){
  return read_string_leng_from_address(address) > 0;
}


bool store_wifi_ssid(String ssid){
  if(ssid.length() > 0 && ssid.length() < 33){ //min leng = 1, max len 32
    write_string_to_address(WIFI_SSID_ADDRESS, ssid);
    return true;
  } 
  return false;
}

bool has_wifi_ssid(){
  return has_string_on_address(WIFI_SSID_ADDRESS);
}
void delete_wifi_ssid(){
  delete_string_from_address(WIFI_SSID_ADDRESS);
}

String get_wifi_ssid(){
  return read_string_from_address(WIFI_SSID_ADDRESS);
}


bool store_wifi_password(String password){
  if(password.length() > 7 && password.length() < 64){ //min leng = 8, max len 63
    write_string_to_address(WIFI_PWD_ADDRESS, password);
    return true;
  } 
  return false;
}

bool has_wifi_password(){
  return has_string_on_address(WIFI_PWD_ADDRESS);
}
void delete_wifi_password(){
  delete_string_from_address(WIFI_PWD_ADDRESS);
}

String get_wifi_password(){
  return read_string_from_address(WIFI_PWD_ADDRESS);
}


