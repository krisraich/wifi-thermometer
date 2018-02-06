
/*
 * Duration in ms, freqency in Hz;
 */
void beep(int duration, int frequency, int gpioPin){

  //doppelte frequenz in HZ
  int sampleFrequency = frequency * 2;
  //in µS
  int sampleCycleTime = 1000000 / sampleFrequency;

   for (int cylceCount = frequency * duration / 1000; cylceCount > 0; cylceCount--){
    digitalWrite(gpioPin, HIGH);
    delayMicroseconds(sampleCycleTime);
    digitalWrite(gpioPin, LOW);
    delayMicroseconds(sampleCycleTime);
  }

  
}

