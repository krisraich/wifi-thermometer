/*
 * Not used: https://techtutorialsx.com/2017/12/01/esp32-arduino-asynchronous-http-webserver/
 * https://www.arduino.cc/en/Reference/WiFi101BeginAP
 * https://github.com/espressif/arduino-esp32/tree/master/libraries/WiFi/src
 */

const char HTML_HEAD[] = "<!DOCTYPE html><html><head><title>Grillthermometer</title><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"><meta http-equiv=\"refresh\" content=\"10\"></head><body><h1>Grillthermometer</h1>";
const char HTML_FOOTER[] = "<footer>By Kris 2018 &#8226; Pitztaler Grillverein &#8226; <a href=\"https://grillverein.tirol\">grillverein.tirol</a> &#8226; <a href=\"mailto:info@grillverein.tirol\">info@grillverein.tirol</a></footer></body></html>";

bool server_is_running = false, server_has_stopped = false;

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

    client.println("<p>Battery: <strong>");
    client.println(get_battery_percente());
    client.println("%</strong></p>");
  
    client.println(HTML_FOOTER);
}

void sending_favicon(WiFiClient &client){
  client.println("Content-Type: image/x-icon");
  client.println();
  for(const uint8_t &current_byte : bin_favicon){
    client.write(current_byte);
  }
}

void setup_webserver() {
  //server_is_running = true;
  //xTaskCreate(&webserver_ap_task, "webserver_ap_task", FREE_RTOS_STACK_SIZE, NULL, WEBSERVER_TASK_PRIORITY, &webserver_handle);


  AsyncWebServer server(80);

  if (DEBUG) Serial.println("Setup Access point");


#if defined(WIFI_AP_PASSWORD)
  WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASSWORD);
#else
  WiFi.softAP(WIFI_AP_SSID);
#endif

  IPAddress myIP;
  // local_ip,   gateway,   subnet
  do{
    break;
     WiFi.softAPConfig(Ip, Ip, NMask);
     myIP = WiFi.softAPIP();
     if(myIP == Ip) break;
     vTaskDelay(1 / portTICK_PERIOD_MS);
  }while(true);
 

  if (DEBUG) {
    Serial.print("AP IP address: ");
    Serial.println(myIP);
    Serial.println("Starting Webserver");
  }


  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", "Hello World");
  });

  //bug?
  //server.begin();

  
}

