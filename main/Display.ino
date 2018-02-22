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
  //taskENTER_CRITICAL(&display_mutex);
  if(DEBUG) Serial.println("Init Display");
  display.init();
  display.setRotation(3); //font orientation
  //taskEXIT_CRITICAL(&display_mutex);
}

void getBoxCords(int cellNr, int &x, int &y){
  int boxHeight = display.height() / 2;
  int boxWidth = display.width() / 3;
  if(cellNr > 3){
    y = boxHeight;
    x = (cellNr - 4) * boxWidth;
  }else{
    y = 0;
    x = (cellNr - 1) * boxWidth;
  }
  
}

void update_display(){
  //taskENTER_CRITICAL(&display_mutex);
  if(DEBUG){
    Serial.println("----- Display Temps ------");
    for (adc1_channel_t current_channel : ADC_CHANNELS){      
      Serial.println("Sensor input No. " + String(current_channel) + " is " + String(get_temperature_from_channel(current_channel)));
    }
    Serial.println("Battery Voltage is: " + String(get_battery_voltage()));
    Serial.println("--------------------------");
  }

  display.fillScreen(GxEPD_WHITE);

  int boxHeight = display.height() / 2;
  int boxWidth = display.width() / 3;
  int borderWidth = 1;


  display.drawLine(boxWidth,0,boxWidth,display.height(),GxEPD_BLACK);
  display.drawLine(boxWidth*2,0,boxWidth*2,display.height(),GxEPD_BLACK);
  display.drawLine(0,boxHeight,display.width(),boxHeight,GxEPD_BLACK);
  
  display.setTextColor(GxEPD_BLACK);
  display.setFont(&FreeMonoBold9pt7b);
  
  int lineheight = 13;
  int x;
  int y;
  int i = 1;
  for (adc1_channel_t current_channel : ADC_CHANNELS){
    // selecting outputbox
    getBoxCords(i, x, y);
    // draw temp. icon
    display.drawBitmap(x+4, y+lineheight+20, icon_temperatur, 24, 24, GxEPD_BLACK);
    // Heading
    display.setCursor(x+2, y+lineheight); // <-- x --> , y^
    display.print("Temp-" + String(current_channel));
    // Temperatur
    display.setCursor(x+30, y+lineheight+30); // <-- x --> , y^
    display.print(String(get_temperature_from_channel(current_channel)));
    // increment box counter for loop
    i++;
  }
    
  getBoxCords(6, x, y);
  display.setCursor(x+2, y+lineheight); // <-- x --> , y^
  display.print("Bat:" + String(get_battery_voltage(), 1) + "V");

  display.setCursor(x+2, y+lineheight*2); // <-- x --> , y^
  display.print("Bat:" + String(get_battery_percente()) + "%");
  
  display.setCursor(x+2, y+lineheight*3); // <-- x --> , y^
  display.print("Con:|||..");
  
  display.setCursor(x+2, y+lineheight*4); // <-- x --> , y^
  display.print("Lootboyz!");
  
  display.update();

  last_refresh = millis();
  //taskEXIT_CRITICAL(&display_mutex);
}

void show_menu(OPERATION_MODE operation_mode){
  //taskENTER_CRITICAL(&display_mutex);
  if(DEBUG) Serial.println("---- Showing Mode: " + String(operation_mode_to_string(operation_mode))+ " -----");

  print_big_text(operation_mode_to_string(operation_mode), &FreeMonoBold18pt7b);
  //simulate time...
  //display.drawBitmap(gImage_logo_floyd, sizeof(gImage_logo_floyd), GxEPD::bm_invert /* | GxEPD::bm_flip_y */);
  
  last_refresh = millis();
  //taskEXIT_CRITICAL(&display_mutex);
}

void show_shutdown(){
  //taskENTER_CRITICAL(&display_mutex);
  if(DEBUG) Serial.println("----- Shut down ------");
  //print_big_text("Shut down empty", &FreeMonoBold18pt7b);
  //simulate time...
  display.drawBitmap(img_logo, sizeof(img_logo));
  //taskEXIT_CRITICAL(&display_mutex);
}


void show_empty_battery(){
  //taskENTER_CRITICAL(&display_mutex);
  if(DEBUG) Serial.println("----- Battery empty ------");
  display.drawBitmap(img_battery_low, sizeof(img_battery_low),  GxEPD::bm_invert);
  //taskEXIT_CRITICAL(&display_mutex);
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
 

