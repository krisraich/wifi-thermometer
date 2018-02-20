/*
 * Recorder for Temp history
 * Doesnt work in Power-Safe mode (RTC_DATA_ATTR doesnt allow arrays apparently)
 */

#define DATA_X_POINTS 296 //294 Pixel per display

float data_points[5][DATA_X_POINTS]; //for each channel
int current_pos = 0;
int current_cycle = 0;

void setup_recorder(){
  if (DEBUG) Serial.println("Init Recoreder"); 
}

void record_temperatures(){
  if(current_cycle++ % TEMPERATUR_HISTORY_SAMPLE_RATIO == 0){
    if (DEBUG) Serial.println("Saving temperatures on cycle " + String(current_cycle)); 
    int channel_pointer = 0;
    int cycle_pointer = current_pos++ % DATA_X_POINTS;
    
    for (adc1_channel_t current_channel : ADC_CHANNELS){     
      data_points[channel_pointer++][cycle_pointer] = get_temperature_from_channel(current_channel);
    }
  }
}

