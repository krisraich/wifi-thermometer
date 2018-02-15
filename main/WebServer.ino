/*
 *   https://www.arduino.cc/en/Reference/WiFi101BeginAP
 *   https://github.com/me-no-dev/ESPAsyncWebServer/blob/master/examples/ESP_AsyncFSBrowser/ESP_AsyncFSBrowser.ino
 *   https://techtutorialsx.com/2017/05/09/esp32-running-code-on-a-specific-core/
 *   
 *   https://github.com/espressif/arduino-esp32/tree/master/libraries/WiFi/src
 *   
 *   https://techtutorialsx.com/2017/12/01/esp32-arduino-asynchronous-http-webserver/
 */


void setup_webserver(){
  if(DEBUG) Serial.println("Setup Access point");

#if defined(WIFI_AP_PASSWORD)
  WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASSWORD);
#else 
  WiFi.softAP(WIFI_AP_SSID);
#endif  

  // local_ip,   gateway,   subnet
  WiFi.softAPConfig(Ip, Ip, NMask);

  if(DEBUG){
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);
  }

/*
 * assertion "don't call tcp_abort/tcp_abandon for listen-pcbs" failed: file "/Users/ficeto/Desktop/ESP32/ESP32/esp-idf-public/components/lwip/core/tcp.c", line 375, function: tcp_abandon
abort() was called at PC 0x400dfbab on core 1

Backtrace: 0x400881d8:0x3ffd4a40 0x400882d7:0x3ffd4a60 0x400dfbab:0x3ffd4a80 0x400f6efd:0x3ffd4ab0 0x400f6fc9:0x3ffd4ae0 0x400d4174:0x3ffd4b00 0x400f3db5:0x3ffd4b20


 * 
  AsyncWebServer server(80);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", "Hello World");
  });
 
  server.begin();
  */

}

