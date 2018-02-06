/*
 * 
 * https://esp-idf.readthedocs.io/en/v2.0/api/peripherals/adc.html
 * https://github.com/G6EJD/ESP32-ADC-Accuracy-Improvement-function/blob/master/ESP32_ADC_Read_Voltage_Accurate.ino
 * http://i.imgur.com/mTkip9V.png
 * http://randomnerdtutorials.com/esp32-dht11-dht22-temperature-humidity-web-server-arduino-ide/
 */



void setup_adc(){
  adc1_config_width(ADC_WIDTH_12Bit);
}

double getVoltage(adc1_channel_t adcChannel){
 return toVolt(readFromPin(adcChannel));
}

int readFromPin(adc1_channel_t adcChannel){
  //adc1_config_channel_atten(adcChannel, ADC_ATTEN_11db);
  int result = adc1_get_raw(adcChannel);
  //Serial.println(result);
  return result;
}


double toVolt(int reading){
  if(reading < 1 || reading >= 4095) return 0;
  // return -0.000000000009824 * pow(reading,3) + 0.000000016557283 * pow(reading,2) + 0.000854596860691 * reading + 0.065440348345433;
  return -0.000000000000016 * pow(reading,4) + 0.000000000118171 * pow(reading,3)- 0.000000301211691 * pow(reading,2)+ 0.001109019271794 * reading + 0.034143524634089;
} 
