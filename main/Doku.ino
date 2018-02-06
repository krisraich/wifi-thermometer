/*
 * Dead Code and docs
 * 
  By Kris
  Complete Project Details http://randomnerdtutorials.com
  https://techtutorialsx.com/2017/05/09/esp32-running-code-on-a-specific-core/
  https://www.arduino.cc/en/Reference/WiFi101BeginAP
  https://raw.githubusercontent.com/gojimmypi/ESP32/master/images/myESP32%20DevKitC%20pinout.png
  http://randomnerdtutorials.com/esp32-dht11-dht22-temperature-humidity-web-server-arduino-ide/
  http://www.instructables.com/id/IOT-Made-Simple-Playing-With-the-ESP32-on-Arduino-/
  http://randomnerdtutorials.com/esp8266-deep-sleep-with-arduino-ide/
  https://github.com/SensorsIot/ESP32-Deep-Sleep
  https://esp-idf.readthedocs.io/en/v2.0/api/peripherals/adc.html

   
#include <DHTesp.h>
#include <WiFi.h>
#include "DHT.h"

#define DHTTYPE DHT11   // DHT 11



// Replace with your network credentials
const char* ssid     = "open_esp";
//const char* password = "";

WiFiServer server(80);

// DHT Sensor
const int DHTPin = 5;
// Initialize DHT sensor.
DHT dht(DHTPin, DHTTYPE);
// Temporary variables
static char celsiusTemp[7];
static char fahrenheitTemp[7];
static char humidityTemp[7];


const int ledr =  4;      // the number of the LED pin Red
const int ledg =  16;      // the number of the LED pin Gellow
const int ledb =  17;      // the number of the LED pin Bue

const int ledArray[] = {
    ledr,
    ledg,
    ledb
  };



// Client variables
char linebuf[80];
int charcount = 0;



#define TOUTCH_PIN T2 // ESP32 Pin D4

#define LED_PIN 2
int touch_value = 100;

#define ANALOG_PIN_X 36
#define ANALOG_PIN_Y 39

int freq = 5000;
int resolution = 8;


int ledChannelg = 0;
int analog_valueg = 0;
int dutyCycleg = 0;

int ledChannelb = 0;
int analog_valueb = 0;
int dutyCycleb = 0;

void setup() {

  dht.begin();
  
  // initialize the LEDs pins as an output:
  pinMode(ledr, OUTPUT);
  pinMode(ledg, OUTPUT);
  pinMode(ledb, OUTPUT);



  ledcSetup(ledChannelg, freq, resolution);
  ledcAttachPin(ledg, ledChannelb);
  ledcWrite(ledChannelg, dutyCycleg);

  ledcSetup(ledChannelb, freq, resolution);
  ledcAttachPin(ledb, ledChannelb);
  ledcWrite(ledChannelb, dutyCycleb);
  

 digitalWrite (ledb, LOW);
 
  //Initialize serial and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }


  // We start by connecting to a WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Starting AP: ");
  Serial.println(ssid);

  //WiFi.begin(ssid, password); //connect to WiFi
  //WiFi.softAP(ssid, password); //mit pw
  WiFi.softAP(ssid);


  IPAddress Ip(192, 168, 1, 1);
  IPAddress NMask(255, 255, 255, 0);

  // local_ip,   gateway,   subnet
  WiFi.softAPConfig(Ip, Ip, NMask);


  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  /* attempt to connect to Wifi network:
  while (WiFi.status() != WL_CONNECTED) {
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("Hey Bobby");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  * /
  Serial.println("Starting webserver... ");
  server.begin();
}

void loop() {

touch_value = touchRead(TOUTCH_PIN);
  //touch example
 /*
  if (touch_value < 50)
  {
    delay(30);
    if(touch_value < 50) digitalWrite (ledb, HIGH);
  }
  else
  {
    digitalWrite (ledb, LOW);
  }
  delay(10);

  return;
*/


  /* /analog_value =  * ;

  analog_valueg = analogRead(ANALOG_PIN_X);
  dutyCycleg = map(analog_valueg, 0, 4095, 0, 255);
  ledcWrite(ledChannelg, dutyCycleg);

  analog_valueb = analogRead(ANALOG_PIN_Y);
  dutyCycleb = map(analog_valueb, 0, 4095, 0, 255);
  ledcWrite(ledChannelb, dutyCycleb);
  
  delay(100);
  
  /* * /
  Serial.print("X: ");
  Serial.print(analogRead(ANALOG_PIN_1_0));
  Serial.print(", Y: ");
  Serial.println(analogRead(ANALOG_PIN_1_3));
  delay(100);
  /* * /
  
  //return;
  
  // listen for incoming clients
  WiFiClient client = server.available();
  if (client) {
    Serial.println("New client");
    memset(linebuf, 0, sizeof(linebuf));
    charcount = 0;
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        //read char by char HTTP request
        linebuf[charcount] = c;
        if (charcount < sizeof(linebuf) - 1) charcount++;
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {


            float h = dht.readHumidity();
            // Read temperature as Celsius (the default)
            float t = dht.readTemperature();
            // Read temperature as Fahrenheit (isFahrenheit = true)
            float f = dht.readTemperature(true);
            // Check if any reads failed and exit early (to try again).
            if (isnan(h) || isnan(t) || isnan(f)) {
              Serial.println("Failed to read from DHT sensor!");
              strcpy(celsiusTemp,"Failed");
              strcpy(fahrenheitTemp, "Failed");
              strcpy(humidityTemp, "Failed");         
            }
            else{
              // Computes temperature values in Celsius + Fahrenheit and Humidity
              float hic = dht.computeHeatIndex(t, h, false);       
              dtostrf(hic, 6, 2, celsiusTemp);             
              float hif = dht.computeHeatIndex(f, h);
              dtostrf(hif, 6, 2, fahrenheitTemp);         
              dtostrf(h, 6, 2, humidityTemp);
              // You can delete the following Serial.print's, it's just for debugging purposes
              /*
                 
              Serial.print("Humidity: ");
              Serial.print(h);
              Serial.print(" %\t Temperature: ");
              Serial.print(t);
              Serial.print(" *C ");
              Serial.print(f);
              Serial.print(" *F\t Heat index: ");
              Serial.print(hic);
              Serial.print(" *C ");
              Serial.print(hif);
              Serial.print(" *F");
              Serial.print("Humidity: ");
              Serial.print(h);
              Serial.print(" %\t Temperature: ");
              Serial.print(t);
              Serial.print(" *C ");
              Serial.print(f);
              Serial.print(" *F\t Heat index: ");
              Serial.print(hic);
              Serial.print(" *C ");
              Serial.print(hif);
              Serial.println(" *F"); //* /
          } 





          
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          client.println();
          client.println("<!DOCTYPE HTML><html><head>");
          client.println("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"></head>");
          client.println("<h1>ESP32 - Web Server</h1>");
         
          
          if(atoi(celsiusTemp)>=25){
            client.println("<div style=\"color: #930000;\">");
          }
          else if(atoi(celsiusTemp)<25 && atoi(celsiusTemp)>=5){
            client.println("<div style=\"color: #006601;\">");
          }
          else if(atoi(celsiusTemp)<5){
            client.println("<div style=\"color: #009191;\">");
          }
          client.println(celsiusTemp);
          client.println("*C</p><p>");
          client.println(fahrenheitTemp);
          client.println("*F</p></div><p>");
          client.println(humidityTemp);
          client.println("%</p></div>");


          
          client.println("<p>LED #R <a href=\"onr\"><button>ON</button></a>&nbsp;<a href=\"offr\"><button>OFF</button></a></p>");
          client.println("<p>LED #G <a href=\"ong\"><button>ON</button></a>&nbsp;<a href=\"offg\"><button>OFF</button></a></p>");
          client.println("<p>LED #B <a href=\"onb\"><button>ON</button></a>&nbsp;<a href=\"offb\"><button>OFF</button></a></p>");
          client.println("<p>LED PARTY <a href=\"onp\"><button>PARTY</button></a></p>");
          client.println("</html>");
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
          if (strstr(linebuf, "GET /onr") > 0) {
            Serial.println("LED R ON");
            digitalWrite(ledr, HIGH);
          }
          else if (strstr(linebuf, "GET /offr") > 0) {
            Serial.println("LED R OFF");
            digitalWrite(ledr, LOW);
          }
          else if (strstr(linebuf, "GET /ong") > 0) {
            Serial.println("LED G ON");
            digitalWrite(ledg, HIGH);
          }
          else if (strstr(linebuf, "GET /offg") > 0) {
            Serial.println("LED G OFF");
            digitalWrite(ledg, LOW);
          }
          else if (strstr(linebuf, "GET /onb") > 0) {
            Serial.println("LED B ON");
            digitalWrite(ledb, HIGH);
          }
          else if (strstr(linebuf, "GET /offb") > 0) {
            Serial.println("LED B OFF");
            digitalWrite(ledb, LOW);
          }
          else if (strstr(linebuf, "GET /onp") > 0) {
            Serial.println("party");
            short time = 10;
            for(int i = 0; i < 200; i++){
                int current = ledArray[i % 3];
              
                digitalWrite(current, HIGH);
                delay(time);
                digitalWrite(current, LOW);
                delay(time);
            }
            digitalWrite(ledb, HIGH);
  
          }
          // you're starting a new line
          currentLineIsBlank = true;
          memset(linebuf, 0, sizeof(linebuf));
          charcount = 0;
        } else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);

    // close the connection:
    client.stop();
    Serial.println("client disconnected");
  }
}
*/
