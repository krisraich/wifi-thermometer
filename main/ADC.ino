/*
 * https://esp-idf.readthedocs.io/en/v2.0/api/peripherals/adc.html
 * https://github.com/G6EJD/ESP32-ADC-Accuracy-Improvement-function/blob/master/ESP32_ADC_Read_Voltage_Accurate.ino
 * http://i.imgur.com/mTkip9V.png
 * http://randomnerdtutorials.com/esp32-dht11-dht22-temperature-humidity-web-server-arduino-ide/
 * http://www.physik.li/publikationen/Thermistor.pdf
 */


//#define ADC_ATTENUATION ADC_ATTEN_11db

void setup_adc(){
  adc1_config_width(ADC_WIDTH_12Bit);
  
  #if defined(ADC_ATTENUATION) && ADC_ATTENUATION !=  ADC_ATTEN_11db //only set when different from default
    for (adc1_channel_t current_channel : ADC_CHANNELS){    
      adc1_config_channel_atten(current_channel, ADC_ATTENUATION); 
    }
  #endif
}

double read_volt_from_channel(adc1_channel_t adc_channel){
 return convert_to_volt(adc1_get_raw(adc_channel));
}


double convert_to_volt(int reading){
  //Serial.println(reading);
  if(reading < 1 || reading >= 4095) return -1;
  //return -0.000000000009824 * pow(reading,3) + 0.000000016557283 * pow(reading,2) + 0.000854596860691 * reading + 0.065440348345433;
  return -0.000000000000016 * pow(reading,4) + 0.000000000118171 * pow(reading,3)- 0.000000301211691 * pow(reading,2)+ 0.001109019271794 * reading + 0.034143524634089;
} 



void calibrate_adcs(){
  Serial.println("Hold calibration Button for 5 Seconds for calibration");
  delay(1000);
  if(! button_has_been_pressed(4000, ON_BOARD_BUTTON, ON_BOARD_BUTTON_PULLDOWN_MODE)){
    Serial.println("Aborting... load normal routine");
    return;
  }
  
  Serial.println("Now in calibration mode!");
  set_blink_frequency(FAST);
  
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

    for (adc1_channel_t current_channel : ADC_CHANNELS){      
      Serial.print(adc1_get_raw(current_channel));
      Serial.print(" ");
    }
    Serial.println(); 
  }  
}


