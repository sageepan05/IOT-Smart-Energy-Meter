IoT-Based Smart Electricity Energy Meter

A smart energy meter built with an ESP32 and a PZEM-004T v3.0 sensor. It measures electrical parameters in real time and shows them on a 20x4 LCD, a local web dashboard, and the Blynk IoT mobile app. A relay protects the load by cutting power during abnormal voltage or current conditions.

Features:

Real-time measurement of voltage, current, power, energy (kWh), power factor and frequency (updated every 1 second)

20x4 I2C LCD showing live readings and relay status

Automatic protection: relay switches off on over-voltage (> 260 V), under-voltage (< 180 V) or over-current (> 30 A)

Push button on the device to toggle the relay

Local web dashboard served by the ESP32 (readable on mobile and desktop) 

JSON API endpoint (/readings) for integration with other tools 

Blynk IoT cloud monitoring of all parameters from a phone

Fallback access point (ESP32_Meter) if the Wi-Fi connection fails

Hardware:

ESP32 board (+ expansion board)	- Main controller with built-in Wi-Fi

PZEM-004T v3.0 - 	Measures AC voltage, current, power, energy, PF, frequency

20x4 LCD + I2C module	Local display

5V relay module	Load switching / protection

Push button	Relay toggle

MP1495 DC-DC step-down module	Stable supply for the electronics

Wires, DC jack, enclosure	Assembly

Safety: This project connects to mains AC voltage. Keep high-voltage parts properly isolated and insulated, and only wire it if you know how to work safely with mains electricity.

Pin configuration: Signal	ESP32 pin, PZEM RX	GPIO 16, PZEM TX	GPIO 17, Relay	GPIO 14, Button	GPIO 23, LCD (I2C, address 0x27)	SDA / SCL (default)

Software and libraries:

Developed in Arduino IDE (C++). 

Install these libraries:
PZEM004Tv30, LiquidCrystal_I2C, ArduinoJson, Blynk (includes BlynkSimpleEsp32), WiFi and WebServer (included with the ESP32 board package)]

Setup:

Clone this repository and open the .ino file in Arduino IDE (the file must sit inside a folder with the same name).

Create a template in the Blynk console with data streams on virtual pins: V0	Voltage, V1	Current, V2	Power, V3	Energy, V4	Power factor, V5	Frequency, V6	Relay state

Replace the placeholders in the code with your own details:

   #define BLYNK_TEMPLATE_ID "YOUR_TEMPLATE_ID"
   
   #define BLYNK_TEMPLATE_NAME "smart energy meter"
   
   #define BLYNK_AUTH_TOKEN "YOUR_BLYNK_AUTH_TOKEN"
   
   const char* ssid = "YOUR_WIFI_NAME";
   
   const char* password = "YOUR_WIFI_PASSWORD";
   
Select your ESP32 board, upload the code, and open the Serial Monitor at 115200 baud.
The LCD shows the device IP address once connected. Open it in a browser to see the dashboard.

How it works:

The PZEM-004T measures the mains parameters and sends them to the ESP32 over serial.
The ESP32 reads the values every second, checks them against the protection thresholds, and drives the relay.
Readings are shown on the LCD, served on the local web page, and sent to Blynk over Wi-Fi.


LCD page switching with the button

Data logging and consumption history

Fire alarm integration, and supply disconnection based on payment status
