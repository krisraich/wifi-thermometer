/*
 * Recorder for Temp history
 * Doesnt work in Power-Safe mode (RTC_DATA_ATTR doesnt allow arrays apparently)
 */


#define DATA_CHANNELS 5

CircularBuffer<float,TEMPERATUR_HISTORY_VALUES> DATA_HISTORY[DATA_CHANNELS];

int current_cycle = 0; //total calls of record_temperatures, used for reducing calls


void setup_recorder(){
  if (DEBUG) Serial.println("Init Recoreder"); 

  //populate recorder
  for(int i = 0; i < DATA_CHANNELS; i++){
    for(int j = 0; j < TEMPERATUR_HISTORY_VALUES; j++){
      DATA_HISTORY[i].unshift((float)random(20, 150));
    }
  }
  
}

void record_temperatures(){
  if(current_cycle++ % TEMPERATUR_HISTORY_SAMPLE_RATIO == 0){
    if (DEBUG) Serial.println("Saving temperatures on cycle " + String(current_cycle)); 

   
    for (adc1_channel_t current_channel : ADC_CHANNELS){  
      DATA_HISTORY[get_index_of_adc_array(current_channel)].unshift(get_temperature_from_channel(current_channel));   
    }
  }
}

//not pretty but meh
void history_json(adc1_channel_t channel, JsonArray *target){
  CircularBuffer<float, TEMPERATUR_HISTORY_VALUES> *current = &DATA_HISTORY[get_index_of_adc_array(channel)];
  for(int i = 0; i < current->size(); i++){
    target->add((*current)[i]); 
  }
}

