/*
 * https://github.com/loboris/ESP32_ePaper_example
 */

void setup_display(){
  
}

 void display_temps_on_display(){

  if(DEBUG){
    Serial.println("----- DISPLAY OUT ------");
    Serial.println("X Voltage: " + String(getVoltage(ANALOG_PIN_X)) + "V");
    Serial.println("Y Voltage: " + String(getVoltage(ANALOG_PIN_Y)) + "V");
    Serial.println("------------------------");
  }
  
 }

