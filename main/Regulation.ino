/*
 * Temperture regulation
 */

void setup_regulation(){
  if (DEBUG) Serial.println("Init Regulation"); 

  pinMode(CHANNEL_1, OUTPUT);
  pinMode(CHANNEL_2, OUTPUT);

  digitalWrite(CHANNEL_1, LOW);
  digitalWrite(CHANNEL_2, LOW);
  
}

