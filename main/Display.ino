/*
 * https://github.com/loboris/ESP32_ePaper_example
 * https://github.com/ZinggJM/GxEPD
 */


//Display (aus GxEPD_SPI_TestExample.ino)
#include <GxEPD.h>
#include <GxGDEH029A1/GxGDEH029A1.cpp>      // 2.9" b/w
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include <Fonts/FreeMonoBold24pt7b.h>
#include <GxIO/GxIO_SPI/GxIO_SPI.cpp>
#include <GxIO/GxIO.cpp>


// pins_arduino.h, e.g. LOLIN32
//static const uint8_t SS    = 5;
//static const uint8_t MOSI  = 23;
//static const uint8_t MISO  = 19;
//static const uint8_t SCK   = 18;

// GxIO_SPI(SPIClass& spi, int8_t cs, int8_t dc, int8_t rst = -1, int8_t bl = -1);
GxIO_Class io(SPI, SS, 17, 16); // arbitrary selection of 17, 16
// GxGDEP015OC1(GxIO& io, uint8_t rst = D4, uint8_t busy = D2);
GxEPD_Class display(io, 16, 4); // arbitrary selection of (16), 4




void setup_display(){
  
}

 void display_temps_on_display(){

  if(DEBUG){
    Serial.println("----- DISPLAY OUT ------");
    Serial.println("X Voltage: " + String(read_volt_from_channel(ANALOG_PIN_X)) + "V");
    Serial.println("Y Voltage: " + String(read_volt_from_channel(ANALOG_PIN_Y)) + "V");
    Serial.println("------------------------");
  }
  
 }

