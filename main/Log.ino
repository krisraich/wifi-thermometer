/*
 * http://esp-idf.readthedocs.io/en/latest/api-reference/system/log.html
 * To override default verbosity define macro: LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
 */

 void setup_log(){

  if(Serial) Serial.setDebugOutput(true);

  if(DEBUG){
    //enable LOG
    esp_log_level_set("*", ESP_LOG_DEBUG);                    // set all components to ERROR level
  }else{
    esp_log_level_set("*", ESP_LOG_ERROR);                    // set all components to ERROR level
    esp_log_level_set(LOG_TAG_WEBSERVER, ESP_LOG_WARN);       // enable WARN logs from WiFi stack
    esp_log_level_set(LOG_TAG_TOUCH, ESP_LOG_WARN);           // enable WARN logs from WiFi stack
  }

  

 
  ESP_LOGD(LOG_TAG_LOG, "Init Log");
 
 }

