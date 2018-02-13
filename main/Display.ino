/*
 * https://github.com/ZinggJM/GxEPD
 * https://www.waveshare.com/wiki/2.9inch_e-Paper_Module
 * https://github.com/ZinggJM/GxEPD/blob/master/GxGDEH029A1/GxGDEH029A1.h
 * 
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
  if(DEBUG) Serial.println("init display");
  display.init();


  /*
  display.drawBitmap(gImage_logo_4c, sizeof(gImage_logo_4c), GxEPD::bm_normal);
  display.update();
  delay(5000);
*/

  if(get_bootups() & 1 == 1){
    display.drawBitmap(gImage_logo_mono, sizeof(gImage_logo_mono), GxEPD::bm_invert | GxEPD::bm_flip_y);
  }else{
    display.drawBitmap(gImage_logo_mono, sizeof(gImage_logo_mono), GxEPD::bm_normal | GxEPD::bm_flip_y);
  }
  
 
 
  //delay(5000);

  
  //drawCornerTest();
  //delay(5000);
  
/*
  //drawBitmap(const uint8_t *bitmap, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color, int16_t mode = bm_normal);
  display.drawBitmap(gImage_gv_image, 0, 0, 86, 200, GxEPD_BLACK);
  display.update();
  delay(5000);
  
  display.drawBitmap(gImage_gv_image, 0, 0, 86, 200, GxEPD_WHITE);
  display.update();
  delay(5000);
*/

  
 // display.fillScreen(GxEPD_WHITE);
  //display.drawExampleBitmap(gImage_gv_image, 0, 0, GxEPD_WIDTH, GxEPD_HEIGHT, GxEPD_BLACK);
  //display.update();
  //delay(5000);

  
  //display.drawBitmap(0, 0, gImage_gv_image, GxEPD_WIDTH, GxEPD_HEIGHT, GxEPD_BLACK);
  //display.drawBitmap(gImage_gv_image, 0, 0, GxEPD_WIDTH, GxEPD_HEIGHT, GxEPD_BLACK);
  //display.update();
//  */
  
}

 void display_temps_on_display(){

  if(DEBUG){
    Serial.println("----- DISPLAY OUT ------");
    
    
    //Serial.println("X Voltage: " + String(read_volt_from_channel(ANALOG_PIN_X)) + "V");
    //Serial.println("Y Voltage: " + String(read_volt_from_channel(ANALOG_PIN_Y)) + "V");
    Serial.println("------------------------");
  }
  
 }

