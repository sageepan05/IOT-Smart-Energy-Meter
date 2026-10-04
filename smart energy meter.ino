//To access Blynk IoT platform
#define BLYNK_TEMPLATE_ID "TMPL6f0-o8FHU"
#define BLYNK_TEMPLATE_NAME "smart energy meter"
#define BLYNK_AUTH_TOKEN ""YOUR_BLYNK_AUTH_TOKEN""

#include <WiFi.h>                //For Wi_FI connectivity on ESP32
#include <WebServer.h>           //To create webserver for local dashboard
#include <LiquidCrystal_I2C.h>   //To control LCD via I2C
#include <PZEM004Tv30.h>         //To handle PZEM sensor
#include <ArduinoJson.h>         //To handle JSON data format for web communcication
#include <BlynkSimpleEsp32.h>    //To connect with Blynk IoT platform

//Wi-Fi Credentials
const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";

//Pin Configuration
#define PZEM_RX 16     //RX pin for PZEM communication
#define PZEM_TX 17     //RX pin for PZEM communication
#define RELAY_PIN 14   //Pin controlling relay on/off
#define BUTTON_PIN 23  //Pin for control LCD page switch

//Hardware Setup
HardwareSerial PZEMSerial(1);                    //Use hardware serial port 1 for PZEM 
PZEM004Tv30 pzem(PZEMSerial, PZEM_RX, PZEM_TX);  //Initialize PZEM
LiquidCrystal_I2C lcd(0x27, 20, 4);              //Initialize LCD with I2C - address 0*27 - 20 coloumn - 4 rows
WebServer server(80);                            //Create webserver on port 80 (80 - standard http port)

//Thresholds values
float overVoltage = 260.0;
float underVoltage = 180.0;
float overCurrent  = 30.0;

//Variables
float v, c, p, e, pf, f;
bool relayState = true;
unsigned long lastRead = 0;
unsigned long lastButtonPress = 0;   //To prevent button bouncing (multiple triggers)
bool lastButtonState = HIGH;         //Prevoius button state detetcion

void setup() 
{
  Serial.begin(115200);   //Start serial communication at baud rate
  delay(1000);            //wait for 1 second for stability

  lcd.init();                        //Start LCD
  lcd.backlight();
  lcd.clear();                       //Clear text 
  lcd.setCursor(0, 0);
  lcd.print("Smart Energy Meter");   
  lcd.setCursor(0, 1);               //Move to 1st column and 2nd row
  lcd.print("Starting...");

  PZEMSerial.begin(9600, SERIAL_8N1, PZEM_RX, PZEM_TX);   //Initialize PZEM communciation
  pinMode(RELAY_PIN, OUTPUT);                             //Set relay pin as output
  setRelay(true);                                         //Set relay on initially
  pinMode(BUTTON_PIN, INPUT_PULLUP);                      //Set button pinas input with pullup resistor

  lcd.setCursor(0, 2);
  lcd.print("Connecting WiFi...");
  WiFi.begin(ssid, password);                            //Start connecting to Wi-Fi

//wait for Wi-Fi connection for 15 seconds
  unsigned long startAttempt = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < 15000) {
    delay(500);
    Serial.print(".");    //Print dots to serial while connecting
  }

  lcd.clear();
  if (WiFi.status() == WL_CONNECTED)  //Print when Wi-Fi connected
  {
    lcd.setCursor(0, 0);
    lcd.print("WiFi Connected");
    lcd.setCursor(0, 1);
    lcd.print(WiFi.localIP());       //Display local IP address
  } 
  else
  {
    lcd.setCursor(0, 0);
    lcd.print("WiFi Failed!");
    WiFi.softAP("ESP32_Meter");     //Create access point IP if Wi-Fi fails
    lcd.setCursor(0, 1);
    lcd.print("AP IP:");
    lcd.setCursor(0, 2);
    lcd.print(WiFi.softAPIP());    //Display Access point IP 
  }

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);   //Connect to Blynk cloud

  server.on("/", handleRoot);                      //Handle main page
  server.on("/readings", handleReadings);          //Handle JSON data requests
  server.begin();                                  //Start web server

  delay(2000);                                     //Wait for 2 seconds
  lcd.clear();
}

void loop()
{
  server.handleClient();   //Handle incoming web request
  Blynk.run();             //Handle blynk communcation

  if (millis() - lastRead > 1000)    //Read sensor every 1 second
  {
    lastRead = millis();   //Update last reading
    readPZEM();            //Read from snesor
    updateLCD();           //Update LCD
    sendToBlynk();         //Send data to Blynk
  }

  handleButton();   //Check for button press
}

//Function for take reading from PZEM
void readPZEM()
{
  v  = pzem.voltage();   //Get voltage
  c  = pzem.current();
  p  = pzem.power();
  e  = pzem.energy();
  pf = pzem.pf();
  f  = pzem.frequency();

  if (isnan(v) || isnan(c))   //If sensor reading failed
  {
    Serial.println("Error reading PZEM!");
    return;   //Exit 
  }

//Saftery values for over volatage, under voltage and over current
  if (v > overVoltage || v < underVoltage || c > overCurrent)
  {
    setRelay(false);  //Turn relay off for safety
  }
  else
  {
    setRelay(true);   //Turn relay on 
  }
}

//Function for LCD display update
void updateLCD() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("V:");   
  lcd.print(v, 1);     //Display voltage with 1 decimal
  lcd.print("V  I:");
  lcd.print(c, 2);     ////Display current with 2 decimal
  lcd.print("A");

  lcd.setCursor(0, 1);
  lcd.print("P:");
  lcd.print(p, 1);
  lcd.print("W  PF:");
  lcd.print(pf, 2);

  lcd.setCursor(0, 2);
  lcd.print("F:");
  lcd.print(f, 1);
  lcd.print("Hz  E:");
  lcd.print(e, 2);
  lcd.print("kWh");

  lcd.setCursor(0, 3);
  lcd.print("Relay: ");
  lcd.print(relayState ? "ON " : "OFF");
}

//Function for relay control
void setRelay(bool state)
{
  relayState = state;
  digitalWrite(RELAY_PIN, state ? HIGH : LOW);   //Set relay pin high for on and low for off
}

//Function for handle web dashboard - display data in website
void handleRoot()
{
  String html = "<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width,initial-scale=1'>";
  html += "<title>ESP32 Energy Meter</title></head><body>";
  html += "<h2>Smart Energy Meter</h2>";
  html += "<p><b>Voltage:</b> " + String(v, 1) + " V</p>";
  html += "<p><b>Current:</b> " + String(c, 2) + " A</p>";
  html += "<p><b>Power:</b> " + String(p, 1) + " W</p>";
  html += "<p><b>Energy:</b> " + String(e, 2) + " kWh</p>";
  html += "<p><b>Power Factor:</b> " + String(pf, 2) + "</p>";
  html += "<p><b>Frequency:</b> " + String(f, 1) + " Hz</p>";
  html += "<p><b>Relay:</b> " + String(relayState ? "ON" : "OFF") + "</p>";
  html += "</body></html>";
  server.send(200, "text/html", html);   //SEnd HTML response
}

//Function - web API which provide sensor reading in JSON format
void handleReadings() 
{
  StaticJsonDocument<256> doc;   //Create JSON document with 256 byte memory allocation
  doc["voltage"] = v;            //Add alreading to JSON object
  doc["current"] = c;
  doc["power"]   = p;
  doc["energy"]  = e;
  doc["pf"]      = pf;
  doc["freq"]    = f;
  doc["relay"]   = relayState;

  String json;
  serializeJson(doc, json);                     //Covert JSON to string
  server.send(200, "application/json", json);   //Send JSON response
}

//Function for send data to Blynk
void sendToBlynk() 
{
  Blynk.virtualWrite(V0, v);   //Virtual pins
  Blynk.virtualWrite(V1, c);
  Blynk.virtualWrite(V2, p);
  Blynk.virtualWrite(V3, e);
  Blynk.virtualWrite(V4, pf);
  Blynk.virtualWrite(V5, f);
  Blynk.virtualWrite(V6, relayState);
}

//Function for button logic
void handleButton()
{
  bool currentState = digitalRead(BUTTON_PIN);   //Read current button state
  if (currentState == LOW && lastButtonState == HIGH && millis() - lastButtonPress > 300)   //Check for button press and also for debounce
  {
    setRelay(!relayState);        //Toggle relay state
    lastButtonPress = millis();   //Record time of button press
  }
  lastButtonState = currentState;   //Update prevoius button state
}