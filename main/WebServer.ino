/*
 * https://github.com/me-no-dev/ESPAsyncWebServer
 * https://arduinojson.org/api/
 * https://www.arduino.cc/en/Reference/WiFi101BeginAP
 * https://github.com/espressif/arduino-esp32/tree/master/libraries/WiFi/src
 * https://getbootstrap.com/docs/4.0/components/alerts/
 * https://arduinojson.org/assistant/?utm_source=github&utm_medium=readme
 */

void setup_webserver() {
  if (DEBUG) Serial.println("Setup Access point");

  String my_ssid = has_wifi_ssid() ? get_wifi_ssid() : DEFAULT_WIFI_SSID;
  if(has_wifi_password()){
    String ap_pwd = get_wifi_password();
    if(DEBUG){
      Serial.print("Accesspoint Password: ");
      Serial.println(ap_pwd);
    }
    WiFi.softAP(my_ssid.c_str(), ap_pwd.c_str(), DEFAULT_WIFI_CHANNEL);
  }else{
    WiFi.softAP(my_ssid.c_str(), NULL, DEFAULT_WIFI_CHANNEL);
  }
  

  IPAddress myIP;
  // local_ip,   gateway,   subnet
  do{
     WiFi.softAPConfig(Ip, Ip, NMask);
     myIP = WiFi.softAPIP();
     if(myIP == Ip) break;
     vTaskDelay(1 / portTICK_PERIOD_MS);
  }while(true);
 

  if (DEBUG) {
    Serial.print("Accesspoint SSID: ");
    Serial.println(my_ssid);
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

      //root element
      JsonObject &root = jsonBuffer.createObject();
      root["battery"] = get_battery_percente();
      root["battery_raw"] = adc1_get_raw(BATTERY_VOLTAGE_ANALOG_IN);

      //create channels array in root element
      JsonArray& channels = root.createNestedArray("channels");
      
      for (ADC_CHANNEL channel : ADC_CHANNELS){

        if(! channel_is_active(channel.channel)) continue;
        
        //create single channel object in channels
        JsonObject& current_channel = channels.createNestedObject();
        current_channel["name"] = channel.name;
        current_channel["temperature"] = get_temperature_from_channel(channel.channel);
        current_channel["adc_raw"] = adc1_get_raw(channel.channel);

        //create channel history array in current channel
        JsonArray& channel_history = current_channel.createNestedArray("history");
        history_json(channel, &channel_history); 
      }
      
      root.printTo(*response);
      request->send(response);
      
  });

  server.on("/getLanguage", HTTP_GET, [](AsyncWebServerRequest *request){
      AsyncResponseStream *response = request->beginResponseStream("text/json");
      DynamicJsonBuffer jsonBuffer;


      JsonObject& root = jsonBuffer.createObject();

      JsonObject& lang_default = root.createNestedObject("default");
      JsonObject& lang_de = root.createNestedObject("de");
      for (TRANSLATION translation : TRANSLATIONS){
        lang_default[translation.identifier] = translation.default_lang;
        lang_de[translation.identifier] = translation.de;
        
      }
      
      root.printTo(*response);
      request->send(response);
      
  });

  

  //make this generic

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

  //chart js
  server.on("/chart.js", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncWebServerResponse *response = request->beginResponse_P(200, "application/javascript", chart_bundle_min_js, chart_bundle_min_js_len);
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });

  //mustace js
  server.on("/mustace.js", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncWebServerResponse *response = request->beginResponse_P(200, "application/javascript", mustache_min_js, mustache_min_js_len);
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });

  //cookie js
  server.on("/cookie.js", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncWebServerResponse *response = request->beginResponse_P(200, "application/javascript", js_cookie_js, js_cookie_js_len);
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });

  if (DEBUG) {
    Serial.print("Starting Webserver with IP:");
    Serial.println(myIP);
  }
  server.begin();
  
}


