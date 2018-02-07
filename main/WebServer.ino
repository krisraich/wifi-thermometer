/*
 *   https://www.arduino.cc/en/Reference/WiFi101BeginAP
 *   https://github.com/me-no-dev/ESPAsyncWebServer/blob/master/examples/ESP_AsyncFSBrowser/ESP_AsyncFSBrowser.ino
 */

 //webserver
#include <AsyncTCP.h>
#include <AsyncEventSource.h>
#include <AsyncJson.h>
#include <AsyncWebSocket.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFSEditor.h>
#include <StringArray.h>
#include <WebAuthentication.h>
#include <WebHandlerImpl.h>
#include <WebResponseImpl.h>
#include <WiFi.h>

