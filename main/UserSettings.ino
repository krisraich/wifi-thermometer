/*
 * Used for communication over serial
 * 
 * to start calibration send string: frist char = mode, other chars = data. Example: 2testpasswd
 *  '1' Calibrate ADC
 *  '2' Set Wifi password
 *  '3' Remove WiFi pwd
 *  '4' Set Wifi SSID
 *  Every thing else will exit calibration
 */

 void clear_serial(){
  while(Serial.available())
    Serial.read();
}


String read_string_from_serial(){
  String b = "";
  byte incomingByte = SERIAL_STOP;
    while(true){
      incomingByte = Serial.read();
      if(incomingByte == SERIAL_STOP) return b;
      b += ((char) Serial.read());
    }
}

 void write_user_settings(){
    if (!Serial) {
    Serial.begin(115200);
    while (!Serial) {
      delay(10); // wait for serial port to connect. Needed for native USB port only
    }
  }


  if(DEBUG) Serial.println("Waiting for calibration script...");

  byte incomingByte = SERIAL_STOP;

  //wait for 1 sec
  for (int i = 0; i < 1000 && Serial && incomingByte == SERIAL_STOP; i++){
      incomingByte = Serial.read();
      delay(1);
  }


  if(incomingByte == SERIAL_STOP){
    if(DEBUG) Serial.println("No input. Continue loading normal routine");
    return;
  }

  set_blink_frequency(FAST);
  print_big_text("Writing settings", &FreeMonoBold18pt7b);

  Serial.println("true");
  
  while(true){
    switch(incomingByte){
      case 49: //ASCII "1"  ADC Calibration mode
        calibrate_adcs();
        break;
        
      case 50: //ASCII "2"  Set WiFi Passwd);
        if(DEBUG) Serial.println("store pw");
        store_wifi_password(read_string_from_serial());
        break;
        
      case 51: //ASCII "3" remove wifi passwd
        delete_wifi_password();
        break;

      case 52: //ASCII "4" set wifi ssi
        store_wifi_ssid(read_string_from_serial());
        break;
      

      case 48: //ASCII "0" 
      default:
        Serial.println("false");
        if(DEBUG) Serial.println("Leaving calibration");
        return; //leave user settings
      
    }
    clear_serial();
    Serial.println("true");

   //wait until something arrives
    do{
      incomingByte = Serial.read();
      delay(1);
    }while(incomingByte == SERIAL_STOP);
  }

  
}









