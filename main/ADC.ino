/*
 * 
 * https://esp-idf.readthedocs.io/en/v2.0/api/peripherals/adc.html
 * https://github.com/G6EJD/ESP32-ADC-Accuracy-Improvement-function/blob/master/ESP32_ADC_Read_Voltage_Accurate.ino
 * http://i.imgur.com/mTkip9V.png
 * http://randomnerdtutorials.com/esp32-dht11-dht22-temperature-humidity-web-server-arduino-ide/
 */



void setup_adc(){
  adc1_config_width(ADC_WIDTH_12Bit);
  //adc1_config_channel_atten(adcChannel, ADC_ATTEN_11db);
}

double read_volt_from_channel(adc1_channel_t adc_channel){
 return convert_to_volt(adc1_get_raw(adc_channel));
}


double convert_to_volt(int reading){
  Serial.println(reading);
  if(reading < 1 || reading >= 4095) return -1;
  //return -0.000000000009824 * pow(reading,3) + 0.000000016557283 * pow(reading,2) + 0.000854596860691 * reading + 0.065440348345433;
  return -0.000000000000016 * pow(reading,4) + 0.000000000118171 * pow(reading,3)- 0.000000301211691 * pow(reading,2)+ 0.001109019271794 * reading + 0.034143524634089;
} 


bool recursive_button_press_detection(int recursion_level_aka_time){
    if(recursion_level_aka_time == 0) {
      if(DEBUG) Serial.println("Entering calibration, reset to exit");
      return true;
    }
    delay(1000);
    if(digitalRead(ON_BOARD_BUTTON) == ON_BOARD_BUTTON_PULLDOWN_MODE ? 0 : 1){
      if(DEBUG) Serial.println("Load calibration routine in: " + String(recursion_level_aka_time - 1));
      return recursive_button_press_detection(--recursion_level_aka_time);
    }
    return false;
  }


void calibrate_adcs(){
  
  pinMode(ON_BOARD_BUTTON, ON_BOARD_BUTTON_PULLDOWN_MODE ? INPUT_PULLUP : INPUT);
  if(DEBUG) Serial.println("For Calibration, hold down calibration button");
  if(! recursive_button_press_detection(5)){
    return;
  }
  
  adc1_config_width(ADC_WIDTH_12Bit);
  
  int incomingByte; 
  while(Serial){
    //wait until something arrives
    do{
      incomingByte = Serial.read();
      delay(1);
    }while(incomingByte == -1);

    while(Serial.available())
      Serial.read();
    
    for(int i = ADC1_CHANNEL_0; i <= ADC1_CHANNEL_7; i++) {
      if(i == 1 || i == 2) continue;
      
      Serial.print(adc1_get_raw(static_cast<adc1_channel_t>(i)));
      Serial.print(" ");
    }
    Serial.println(); 
  }  
}


