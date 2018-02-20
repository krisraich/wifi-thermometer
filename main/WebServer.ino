/*
 * Not used: https://techtutorialsx.com/2017/12/01/esp32-arduino-asynchronous-http-webserver/
 * https://www.arduino.cc/en/Reference/WiFi101BeginAP
 * https://github.com/espressif/arduino-esp32/tree/master/libraries/WiFi/src
 */

const char HTML_HEAD[] = "<!DOCTYPE html><html><head><title>Grillthermometer</title><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"><meta http-equiv=\"refresh\" content=\"10\"></head><body><h1>Grillthermometer</h1>";
const char HTML_FOOTER[] = "<footer>By Kris 2018 &#8226; Pitztaler Grillverein &#8226; <a href=\"https://grillverein.tirol\">grillverein.tirol</a> &#8226; <a href=\"mailto:info@grillverein.tirol\">info@grillverein.tirol</a></footer></body></html>";

void sending_html(WiFiClient &client){
   // send a standard http response header
    client.println("Content-Type: text/html");
    client.println();
    client.println(HTML_HEAD);

     for (adc1_channel_t current_channel : ADC_CHANNELS){      
      client.println("<p>Sensor input No. ");
      client.println(current_channel);
      client.println(" is <strong>");
      client.println(adc1_get_raw(current_channel));
      client.println("</strong></p>");
    }

    client.println("<p>Battery Voltage is: <strong>");
    client.println(get_battery_voltage());
    client.println("V</strong></p>");
  
    client.println(HTML_FOOTER);
}

void sending_favicon(WiFiClient &client){
  client.println("Content-Type: image/x-icon");
  client.println();
  for(const uint8_t &current_byte : logo_favicon){
    client.write(current_byte);
  }
}

void webserver_ap_task(void *pvParameter) {

  if (DEBUG) Serial.println("Setup Access point");

#if defined(WIFI_AP_PASSWORD)
  WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASSWORD);
#else
  WiFi.softAP(WIFI_AP_SSID);
#endif

  IPAddress myIP;
  // local_ip,   gateway,   subnet
  do{
     WiFi.softAPConfig(Ip, Ip, NMask);
     myIP = WiFi.softAPIP();
     Serial.println(myIP);
     if(myIP == Ip) break;
     vTaskDelay(1 / portTICK_PERIOD_MS);
  }while(true);
 

  if (DEBUG) {
    Serial.print("AP IP address: ");
    Serial.println(myIP);
    Serial.println("Starting Webserver");
  }

  WiFiServer web_server(80);
  web_server.begin();

  char linebuf[80];
  int charcount = 0;

  while (true) {
    // listen for incoming clients
    WiFiClient client = web_server.available();
    if (client) {
      
      if (DEBUG) {
        IPAddress clientIP = client.remoteIP();
        Serial.print("New client with IP: ");
        Serial.println(clientIP);
      }
      
      memset(linebuf, 0, sizeof(linebuf));
      charcount = 0;
      // an http request ends with a blank line
      boolean currentLineIsBlank = true;
      while (client.connected()) {
        if (client.available()) {
          
          char c = client.read(); //read byte for byte...
          //Serial.write(c);
          //read char by char HTTP request
          linebuf[charcount] = c;
          if (charcount < sizeof(linebuf) - 1) charcount++;
          
          // if you've gotten to the end of the line (received a newline
          // character) and the line is blank, the http request has ended,
          // so you can send a reply
          if (c == '\n' && currentLineIsBlank) {

            client.println("HTTP/1.1 200 OK");
            client.println("Connection: close");
            
            if (strstr(linebuf, "GET /favicon.ico") > 0) {
              Serial.println("Sending favicon");
              sending_favicon(client);
            }else{
              Serial.println("Sending website");
              sending_html(client);
            }
           
            //MUST flush data... otherwise no output
            client.flush();
            break;
          }
          if (c == '\n') {
            // you're starting a new line
            currentLineIsBlank = true;

          } else if (c != '\r') {
            // you've gotten a character on the current line
            currentLineIsBlank = false;
          }
        }
      }
      // give the web browser time to receive the data
      vTaskDelay(1 / portTICK_PERIOD_MS);

      // close the connection:
      client.stop();
      //if (DEBUG) Serial.println("client processed");
    }
  }
}


void setup_webserver() {
  xTaskCreate(&webserver_ap_task, "webserver_ap_task", 2048, NULL, WEBSERVER_TASK_PRIORITY, &webserver_handle);
}

