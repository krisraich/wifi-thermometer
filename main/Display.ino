/*
 * https://github.com/ZinggJM/GxEPD
 * https://www.waveshare.com/wiki/2.9inch_e-Paper_Module
 * https://github.com/ZinggJM/GxEPD/blob/master/GxGDEH029A1/GxGDEH029A1.h
 * https://github.com/ZinggJM/GxEPD/blob/master/examples/GxEPD_SPI_TestExample/GxEPD_SPI_TestExample.ino
 * https://github.com/adafruit/Adafruit-GFX-Library/tree/master/Fonts
 * 
 * display.drawBitmap(gImage_logo_floyd, sizeof(gImage_logo_floyd), GxEPD::bm_invert /* | GxEPD::bm_flip_y * /);
 */


void drawCornerTest()
{
  display.drawCornerTest();
  delay(5000);
  uint8_t rotation = display.getRotation();
  for (uint16_t r = 0; r < 4; r++)
  {
    display.setRotation(r);
    display.fillScreen(GxEPD_WHITE);
    display.fillRect(0, 0, 8, 8, GxEPD_BLACK);
    display.fillRect(display.width() - 18, 0, 16, 16, GxEPD_BLACK);
    display.fillRect(display.width() - 25, display.height() - 25, 24, 24, GxEPD_BLACK);
    display.fillRect(0, display.height() - 33, 32, 32, GxEPD_BLACK);
    display.update();
    delay(5000);
  }
  display.setRotation(rotation); // restore
}


void setup_display(){
  if(DEBUG) Serial.println("Initialize display");
  display.init();
  display.setRotation(3); //font orientation
}

void update_display(){

  if(DEBUG){
    Serial.println("----- Display Temps ------");
    for (adc1_channel_t current_channel : ADC_CHANNELS){      
      Serial.println("Sensor input No. " + String(current_channel) + " is " + String(adc1_get_raw(current_channel)));
    }
    Serial.println("--------------------------");
  }
  
 }

 void show_menu(){
  if(DEBUG) Serial.println("----- Display Menu ------");
 }

/*
 * f: FreeMonoBold9pt7b FreeMonoBold12pt7b FreeMonoBold18pt7b FreeMonoBold24pt7b
 */
void print_big_text(const char text[], const GFXfont* f){
  display.fillScreen(GxEPD_WHITE);
  display.setTextColor(GxEPD_BLACK);
  display.setFont(f);
  display.setCursor(10, display.height() / 2 + 12); // <-- x --> , y^
  display.print(text);
  display.update();
}
 

