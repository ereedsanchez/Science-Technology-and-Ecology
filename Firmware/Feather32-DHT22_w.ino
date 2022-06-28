// Example testing sketch for Technology Ecology Feather ESP32 Board with
//with DHT22 senor
// Written by Edwin Reed-Sanchez

// REQUIRES the following Arduino libraries:
// - DHT Sensor Library: https://github.com/adafruit/DHT-sensor-library
// - Adafruit Unified Sensor Lib: https://github.com/adafruit/Adafruit_Sensor


#include "DHT.h"
#include <OneWire.h> 
#include <DallasTemperature.h>

#define DHTPIN 14     // Digital pin connected to the DHT sensor
// Uncomment whatever type you're using!
//#define DHTTYPE DHT11   // DHT 11
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

#define ONE_WIRE_PIN 32

// Connect pin 1 3.3V logic
// Connect pin 2 of the sensor to whatever your DHTPIN is
// Connect pin 3 (on the right) of the sensor to GROUND (if your sensor has 3 pins)
// Connect pin 4 (on the right) of the sensor to GROUND and leave the pin 3 EMPTY (if your sensor has 4 pins)
// Connect a 10K resistor from pin 2 (data) to pin 1 (power) of the sensor

// Initialize DHT sensor.
DHT dht(DHTPIN, DHTTYPE);


// GPIO where the DS18B20 is connected to
const int oneWireBus = 32;     

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(oneWireBus);

//OneWire oneWire(ONE_WIRE_PIN);
//OneWire ds(32);
DallasTemperature sensors(&oneWire);

//DHTesp dht;

//const char* ssid = "NET-MESH-FOREST";                //Nombre de la RED
//const char* password = "B4r3f2c1!+";           //Password de la RED
//const char* mqtt_server = "192.168.128.124";   //Dirección servidor
//const char* http_server = "192.168.0.148";
//const char* HOSTNAME = "CritterCam_1";

//WiFiClient espClient;
//long lastMsg = 0;
//char msg[100];
//int value = 0;
//int sensorPin = A0;


void setup() {
  Serial.begin(9600);
  Serial.println(F("DHTxx test!"));
  dht.begin();
}

void loop() {
  // Wait a few seconds between measurements.
  delay(2000);

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

// Read from DS1820
    float temperatureC = sensors.getTempCByIndex(0);
  float temperatureF = sensors.getTempFByIndex(0);

  

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.print(F("°C "));
  Serial.print(f);
  Serial.print(F("°F  Heat index: "));
  Serial.print(hic);
  Serial.print(F("°C "));
  Serial.print(hif);
  Serial.print(F("°F"));
  Serial.print(" TEMP-DS1820 ");
  Serial.print(temperatureC);
  Serial.print(" C ");
  Serial.print(temperatureF);
  Serial.println(" F ");
  ;
}
