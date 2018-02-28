/*
 * https://esp-idf.readthedocs.io/en/v2.0/api/peripherals/adc.html
 * https://github.com/G6EJD/ESP32-ADC-Accuracy-Improvement-function/blob/master/ESP32_ADC_Read_Voltage_Accurate.ino
 * http://i.imgur.com/mTkip9V.png
 * http://randomnerdtutorials.com/esp32-dht11-dht22-temperature-humidity-web-server-arduino-ide/
 * http://www.physik.li/publikationen/Thermistor.pdf
 */


//#define ADC_ATTENUATION ADC_ATTEN_11db


REGRESSION_PARAMETER current_params;

float get_battery_voltage(){
  return get_adc_raw_voltage() * (BATTERY_VOLTAGE_DEVIDING_RESISTOR_1 + BATTERY_VOLTAGE_DEVIDING_RESISTOR_2) / BATTERY_VOLTAGE_DEVIDING_RESISTOR_2;
} 

float get_adc_raw_voltage(){
  int reading = adc1_get_raw(BATTERY_VOLTAGE_ANALOG_IN);
  
  if(reading < 1 || reading >= 4095) return -1;
  //return -0.000000000009824 * pow(reading,3) + 0.000000016557283 * pow(reading,2) + 0.000854596860691 * reading + 0.065440348345433;
  return -0.000000000000016 * pow(reading,4) + 0.000000000118171 * pow(reading,3)- 0.000000301211691 * pow(reading,2)+ 0.001109019271794 * reading + 0.034143524634089;
} 

float get_temperature_from_channel(adc1_channel_t current_channel){
  return adc1_get_raw(current_channel) / 100; //dummy
}

uint8_t get_battery_percente(){
  return (get_battery_voltage() - MINIMUM_BATTERY_VOLTAGE) / (MAX_BATTERY_VOLTAGE - MINIMUM_BATTERY_VOLTAGE) * 100;
}

void setup_adc(){
  adc1_config_width(ADC_WIDTH_12Bit);
  
  #if defined(ADC_ATTENUATION) && ADC_ATTENUATION !=  ADC_ATTEN_11db //only set when different from default
    for (adc1_channel_t current_channel : ADC_CHANNELS){    
      adc1_config_channel_atten(current_channel, ADC_ATTENUATION); 
    }
  #endif

  //set up battery reader
  if(DEBUG){
    Serial.print("Setup ADC, Battery voltage: ");
    Serial.println(get_battery_voltage());
  }

  //load params
  current_params = read_regression_params();
  Serial.println(current_params.param_a);
  Serial.println(current_params.param_b);
  Serial.println(current_params.param_c);
  Serial.println(current_params.param_d);
  
}

void clear_serial(){
  while(Serial.available())
    Serial.read();
}

void calibrate_adcs(){
  

  if (!Serial) {
    Serial.begin(115200);
    while (!Serial) {
      delay(10); // wait for serial port to connect. Needed for native USB port only
    }
  }


  if(DEBUG) Serial.println("Waiting for calibration script...");

  int incomingByte = -1; 

  //wait for 1 sec
  for (int i = 0; i < 1000 && Serial && incomingByte == -1; i++){
      incomingByte = Serial.read();
      delay(1);
  }

  REGRESSION_PARAMETER tmp = {
    1.1,
    2.22,
    3.333,
    4.4444,
  };

  save_regression_params(tmp);

  
  
  if(incomingByte == -1){
    if(DEBUG) Serial.println("No input. Continue loading normal routine");
    return;
  }else{
    clear_serial();
    Serial.println("true");
  }

  //set powersave mode (failsafe)
  save_operation_mode(POWER_SAVING);
  
  set_blink_frequency(FAST);
  print_big_text("CALIBRATI0N!1", &FreeMonoBold18pt7b); 
  
  
  
  while(Serial && incomingByte != 3){ // 3 = end of text
    //wait until something arrives
    do{
      incomingByte = Serial.read();
      delay(1);
    }while(incomingByte == -1);

    clear_serial();

    for (adc1_channel_t current_channel : ADC_CHANNELS){      
      Serial.print(adc1_get_raw(current_channel));
      Serial.print(" ");
    }
    Serial.println(); 
  }  

  //ToDo: read param, and store in eeprom
  
}


