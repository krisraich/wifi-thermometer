/*
 * Used for communication over serial in WiFi Mode
 * 
 * to start calibration send string: frist char = mode, other chars = data. Example: 2testpasswd
 *  '1' Calibrate ADC
 *  '2' Set Wifi password
 *  '3' Remove WiFi pwd
 *  '4' Set Wifi SSID
 *  Every thing else will exit calibration
 */

void user_settings_task(void *pvParameter) {
  byte incomingByte = SERIAL_STOP;
  String out;
   
  while(true){
    incomingByte = Serial.read();
  
    if(incomingByte != SERIAL_STOP){
      //somthing was received
      switch(incomingByte){         
        case 48: //ASCII "0"  Read ADC
        {
          out = "[";
          for (ADC_CHANNEL current_channel : ADC_CHANNELS){
            out += String(adc1_get_raw(current_channel.channel)) + ",";
          }
          out.remove(out.length() - 1);
          out += "]";
          Serial.println(out);
          break;
        }

        case 49: //ASCII "1" Read Log Value
        {
          out = "[";
          for (ADC_CHANNEL current_channel : ADC_CHANNELS){
            out += String(get_log_value_from_channel(current_channel.channel)) + ",";
          }
          out.remove(out.length() - 1);
          out += "]";
          Serial.println(out);
          break;
        }

          
        case 50: //ASCII "2"  Set WiFi Passwd;
        {
          if(DEBUG) Serial.print("Calibration: Set WiFi Password to: ");
          String wifipw = read_string_from_serial();
          if(DEBUG) Serial.println(wifipw);
          store_wifi_password(wifipw);
          break;
        }
          
        case 51: //ASCII "3" remove wifi passwd
        {
          if(DEBUG) Serial.println("Calibration: Delete WiFi Password");
          delete_wifi_password();
          break;
        }

  
        case 52: //ASCII "4" set wifi ssi
        {
          if(DEBUG) Serial.print("Calibration: Set WiFi SSID to: ");
          String wifissid = read_string_from_serial();
          if(DEBUG) Serial.println(wifissid);
          store_wifi_ssid(wifissid);
          break;
        }

        
        default:
        {
          if(DEBUG){
            Serial.print("Calibration: Unknowen Mode (char / bytevalue): ");
            Serial.print((char)incomingByte);
            Serial.print(" / ");
            Serial.println(incomingByte);
          }    
        }
      }
      clear_serial();
    }
   
    vTaskDelay(SERIAL_LOOP_CHECK_TIME * mS_TO_S_FACTOR / portTICK_PERIOD_MS);
  }
}

void setup_user_settings() {
  xTaskCreate(&user_settings_task, "user_settings_task", FREE_RTOS_STACK_SIZE, NULL, USER_SETTINGS_TASK_PRIORITY, &user_settings_handle);
}




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
      b += ((char) incomingByte);
    }
}






