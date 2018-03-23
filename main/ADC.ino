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

bool channel_is_active(adc1_channel_t current_channel){
  int reading = adc1_get_raw(current_channel);
  return !(reading < 1 || reading > 4095);
}



/*
 * temp is calculated with a variation of Steinhart-Hart equation
 * 1/T = a + b(ln(R))+c(ln(R))³
 * 
 * For a better result the following formula is used:
 * 
 * 1/T = a + b(ln(Rt/Rs)) + c(ln(Rt/Rs))² + d(ln(Rt/Rs))³
 * 
 * Rt is the resistance of the Sensor with a given temperature
 * Rs is the resistance of the Default value (eg 1000k @ 25°C)
 * As input the voltage of Rvd is used. (100k)
 * 
 * Since we do not calculate the actual resistance (only the voltage) we can replace these.
 * At 25°C we split Vcc 10:1, assuming the ADC has a linear characteristic curve
 * Therefore Urt and Urvd should have an theoretical ADC value of 410 at 25°C (The ADC hast 12 bit --> max value 4096) 
 * Since we only measure Urs we use (4096 - Urvd) to calculate Urt
 * 
 * We can replace now ln(Rt/Rs) with ln((4096 - Urvd) / 410), we call this value lnr
 * 
 * Stimmt nicht.. fixen!
 * 
 */
double get_temperature_from_channel(adc1_channel_t current_channel){
  double lnr = get_log_value_from_channel(current_channel);
  return 1 / (current_params.param_a + current_params.param_b * lnr + current_params.param_c * pow(lnr, 2) + current_params.param_d * pow(lnr, 3));
}

double get_log_value_from_channel(adc1_channel_t current_channel){
  int raw_adc = adc1_get_raw(current_channel); //Urvd
  return log( (float)((float)(4096 - raw_adc))  / ((float)(4096.0 * VOLTAGE_DEVIDOR_RESISTANCE) / DEFAULT_RESISTANC_20 ));
}

uint8_t get_battery_percente(){
  return (get_battery_voltage() - MINIMUM_BATTERY_VOLTAGE) / (MAX_BATTERY_VOLTAGE - MINIMUM_BATTERY_VOLTAGE) * 100;
}

void setup_adc(){
  adc1_config_width(ADC_WIDTH_12Bit);

  #if defined(ADC_ATTENUATION) && ADC_ATTENUATION !=  ADC_ATTEN_11db //only set when different from default
    for (ADC_CHANNEL current_channel : ADC_CHANNELS){
      adc1_config_channel_atten(current_channel.channel, ADC_ATTENUATION);
    }
  #endif

  analogSetCycles(MEASURE_CYCLES); //default is 8 
  analogSetSamples(MEASURE_SAMPLES); //default is 1
  analogSetClockDiv(MEASURE_CLOCK_DIVIDOR); //default is 1

  //set up battery reader
  if(DEBUG){
    Serial.print("Setup ADC, Battery voltage: ");
    Serial.println(get_battery_voltage());
  }

  //load params
  current_params = read_regression_params();
}



void calibrate_adcs(){

  byte incomingByte;
  while(Serial && incomingByte != 3){
    //wait until something arrives
    do{
      incomingByte = Serial.read();
      delay(1);
    }while(incomingByte == SERIAL_STOP);

    clear_serial();

    if(incomingByte == 35) break; // ASCII "#" = go to nex step

    for (ADC_CHANNEL current_channel : ADC_CHANNELS){
      Serial.print(adc1_get_raw(current_channel.channel));
      Serial.print(" ");
    }
    Serial.println();
  }

  Serial.println("true");

  //read param, and store in eeprom
  float params[4];
  float *ptr;
  ptr = params;

  int floats_read = 0;
  String float_parts = "";

  while(Serial){
    //wait until something arrives
    do{
      incomingByte = Serial.read();
      delay(1);
    }while(incomingByte == SERIAL_STOP);

    if(incomingByte == 32 && floats_read < 4){ 
      *ptr = float_parts.toFloat(); //parse string to float and write to pointer
      ptr++; //inc array pointer
      float_parts = ""; //empty buffer
      floats_read++; //prevent mem leaks
    }else if(incomingByte == 35){  //ASCII "#" = go to nex step
        break;
    }else{
      float_parts += String((char)incomingByte);
    }
  }

  REGRESSION_PARAMETER tmp = {
    params[0],
    params[1],
    params[2],
    params[3],
  };

  Serial.println(print_regression_parameter(tmp));
  save_regression_params(tmp);

  current_params = tmp;

}
