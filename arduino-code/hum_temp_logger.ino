#include "WiFi.h"
#include <HTTPClient.h>
#include <SHT1x.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// Specify data and clock connections and instantiate SHT1x object
#define dataPin  12
#define clockPin 13
SHT1x sht1x(dataPin, clockPin);
//----------------------------------------

// Defining LED PINs on the ESP32 Board.
#define On_Board_LED_PIN  2

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);


//----------------------------------------SSID and PASSWORD of your WiFi network.
const char* ssid = "";  //--> Your wifi name
const char* password = ""; //--> Your wifi password
//----------------------------------------

// Google script Web_App_URL.
String Web_App_URL = "https://script.google.com/macros/s/AKfycbzeJuQ6tR-_erFKArkx-MR1fBmBCAd-dMVvdt6H8zi0zkJ5_mURwhatA6VYJUJn2HGV0Q/exec";


String Status_Read_Sensor = "";
float Temp;
int Humd;


void Getting_DHT10_Sensor_Data() {
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  Humd = sht1x.readHumidity();
  // Read temperature as Celsius (the default)
  Temp = sht1x.readTemperatureC();

  // Check if any reads failed and exit early (to try again).
  if (isnan(Humd) || isnan(Temp)) {
    Serial.println();
    Serial.println(F("Failed to read from DHT sensor!"));
    Serial.println();

    Status_Read_Sensor = "Failed";
    Temp = 0.00;
    Humd = 0;
  } 
  else {
    Status_Read_Sensor = "Success";
  }

  Serial.println();
  Serial.println("-------------");
  Serial.print(F("Status_Read_Sensor : "));
  Serial.print(Status_Read_Sensor);
  Serial.print(F(" | Humidity : "));
  Serial.print(Humd);
  Serial.print(F("% | Temperature : "));
  Serial.print(Temp);
  Serial.println(F("°C"));
  Serial.println("-------------");

  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Temp: ");
  display.print(Temp);
  display.setCursor(0, 10);
  display.print("Humidity: ");
  display.print(Humd);
  display.display();
}

//________________________________________________________________________________VOID SETUP()
void setup() {
  // put your setup code here, to run once:

  Serial.begin(115200);

  delay(2000); // Pause for 2 seconds

  Wire.begin(5, 4);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C, false, false)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  display.setCursor(0, 0);

  delay(1000);


  pinMode(On_Board_LED_PIN, OUTPUT);

  //----------------------------------------Set Wifi to STA mode
  Serial.println();
  Serial.println("-------------");
  Serial.println("WIFI mode : STA");
  WiFi.mode(WIFI_STA);
  Serial.println("-------------");
  //---------------------------------------- 

  //----------------------------------------Connect to Wi-Fi (STA).
  Serial.println();
  Serial.println("------------");
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  //:::::::::::::::::: The process of connecting ESP32 with WiFi Hotspot / WiFi Router.
  // The process timeout of connecting ESP32 with WiFi Hotspot / WiFi Router is 20 seconds.
  // If within 20 seconds the ESP32 has not been successfully connected to WiFi, the ESP32 will restart.
  // I made this condition because on my ESP32, there are times when it seems like it can't connect to WiFi, so it needs to be restarted to be able to connect to WiFi.

  int connecting_process_timed_out = 20; //--> 20 = 20 seconds.
  connecting_process_timed_out = connecting_process_timed_out * 2;
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    digitalWrite(On_Board_LED_PIN, HIGH);
    delay(250);
    digitalWrite(On_Board_LED_PIN, LOW);
    delay(250);
    if (connecting_process_timed_out > 0) connecting_process_timed_out--;
    if (connecting_process_timed_out == 0) {
      delay(1000);
      ESP.restart();
    }
  }

  digitalWrite(On_Board_LED_PIN, LOW);
  
  Serial.println();
  Serial.println("WiFi connected");
  
  display.print("WiFi connected!");
  display.display();
  
  Serial.println("------------");
  //::::::::::::::::::
  //----------------------------------------

  delay(100);

  Serial.println();
  Serial.println("DHT10 Begin");
  Serial.println();
  delay(1000);
  

  delay(2000);
}
//________________________________________________________________________________

//________________________________________________________________________________VOID LOOP()
void loop() {
  // put your main code here, to run repeatedly:

  // Calling the "Getting_DHT10_Sensor_Data()" subroutine.
  Getting_DHT10_Sensor_Data();

  //----------------------------------------Conditions that are executed when WiFi is connected.
  // This condition is the condition for sending or writing data to Google Sheets.
  if (WiFi.status() == WL_CONNECTED) {
    digitalWrite(On_Board_LED_PIN, HIGH);

    // Create a URL for sending or writing data to Google Sheets.
    String Send_Data_URL = Web_App_URL + "?sts=write";
    Send_Data_URL += "&srs=" + Status_Read_Sensor;
    Send_Data_URL += "&temp=" + String(Temp);
    Send_Data_URL += "&humd=" + String(Humd);


    Serial.println();
    Serial.println("-------------");
    Serial.println("Send data to Google Spreadsheet...");
    Serial.print("URL : ");
    Serial.println(Send_Data_URL);

    //::::::::::::::::::The process of sending or writing data to Google Sheets.
      // Initialize HTTPClient as "http".
      HTTPClient http;
  
      // HTTP GET Request.
      http.begin(Send_Data_URL.c_str());
      http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  
      // Gets the HTTP status code.
      int httpCode = http.GET(); 
      Serial.print("HTTP Status Code : ");
      Serial.println(httpCode);
  
      // Getting response from google sheets.
      String payload;
      if (httpCode > 0) {
        payload = http.getString();
        Serial.println("Payload : " + payload);    
      }
      
      http.end();
    //::::::::::::::::::
    
    digitalWrite(On_Board_LED_PIN, LOW);
    Serial.println("-------------");
  }
  //----------------------------------------

  delay(10000);
}

