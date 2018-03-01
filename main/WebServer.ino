/*
 * https://github.com/me-no-dev/ESPAsyncWebServer
 * https://arduinojson.org/api/
 * https://www.arduino.cc/en/Reference/WiFi101BeginAP
 * https://github.com/espressif/arduino-esp32/tree/master/libraries/WiFi/src
 * https://getbootstrap.com/docs/4.0/components/alerts/
 */

void setup_webserver() {
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
     if(myIP == Ip) break;
     vTaskDelay(1 / portTICK_PERIOD_MS);
  }while(true);
 

  if (DEBUG) {
    Serial.print("AP IP address: ");
    Serial.println(myIP);
    Serial.println("Starting Webserver");
  }


  //main page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", index_html, index_html_len);
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });

  //temps
  server.on("/getTemps", HTTP_GET, [](AsyncWebServerRequest *request){
      AsyncResponseStream *response = request->beginResponseStream("text/json");
      DynamicJsonBuffer jsonBuffer;
      JsonObject &root = jsonBuffer.createObject();
      root["read"] = true;

      root["battery"] = get_battery_percente();
      
      JsonObject& temps = root.createNestedObject("temps");
      JsonArray& history = root.createNestedArray("history");
      
      for (adc1_channel_t current_channel : ADC_CHANNELS){
        temps[String(current_channel)] = get_temperature_from_channel(current_channel); 

        JsonObject& current_history = history.createNestedObject();
        JsonArray& history_array = current_history.createNestedArray(String(current_channel));
        history_json(current_channel, &history_array); 
      }
      
      root.printTo(*response);
      request->send(response);
      
  });

  //favicon
  server.on("/favicon.png", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncWebServerResponse *response = request->beginResponse_P(200, "image/png", favicon_png, favicon_png_len);
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });
  
  server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request){
    request->redirect("/favicon.png");
  });

  //bootstrap
  server.on("/bootstrap.css", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/css", bootstrap_min_css, bootstrap_min_css_len);
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });

  //jquery
  server.on("/jquery.js", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncWebServerResponse *response = request->beginResponse_P(200, "application/javascript", jquery_3_3_1_min_js, jquery_3_3_1_min_js_len);
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });

  server.begin();
  
}

