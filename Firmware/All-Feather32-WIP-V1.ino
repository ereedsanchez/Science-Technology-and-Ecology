//#include <ESP8266WiFi.h>
#include <WiFi.h>
#include <PubSubClient.h>
//#include <ESP8266HTTPClient.h>
#include <HTTPClient.h>
//#include <DHTesp.h>
#include "DHT.h"
#include <DallasTemperature.h>
#include <OneWire.h>
#include "Adafruit_SHT31.h"

#define DHTPIN 14
#define DHTTYPE DHT22
#define ONE_WIRE_PIN 32

// Initialize DHT sensor.
DHT dht(DHTPIN, DHTTYPE);

bool enableHeater = false;
uint8_t loopCnt = 0;

int value = 0;
float vIN = 0.0;
float vOUT = 0.0;
float R1 = 30000.0;
float R2 = 7500.0;

String _appKey = "BRF-APP";
String _netKey = "BRF-NET";

long period = 120;

const int oneWireBus = 32; 

//OneWire oneWire(D4);
// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(oneWireBus);
OneWire  ds(32);
DallasTemperature sensors(&oneWire);
//DHTesp dht;

Adafruit_SHT31 sht31 = Adafruit_SHT31();


const char* ssid = "NET-MESH-FOREST";              //Nombre de la RED
const char* password = "B4r3f2c1!+";           //Password de la RED
const char* mqtt_server_local = "192.168.0.148";
const char* mqtt_server_remote = "165.22.191.125";
const char* http_collector_server_local = "";
const char* http_collector_server_remote = "";
const char* HOME_TOPIC = "home";

const char* HOSTNAME = "BRF01";
const char* host = "BRF01";

// device_ids for sensors
// THIS ARE THE ADDRESS WE USE UN GRAFANA, THIS ONE ARE READY TO USE 
const char* DHT22TEMP = "630";
const char* DHT22HUM = "640";
const char* SHT30TEMP = "650";
const char* SHT30HUM = "660";
const char* DS18B20TEMP = "670";
const char* VOLTAGE = "680";

const int MAX_ANALOG_VAL = 4095;
const float MAX_BATTERY_VOLTAGE = 4.2; // Max LiPoly voltage of a 3.7 battery is 4.2

WiFiClient espClient;



void ds1820getMeasurements(){
  byte i;
  byte present = 0;
  byte type_s;
  byte data[9];
  byte addr[8];
  float celsius, fahrenheit;
  
  if ( !ds.search(addr)) {
    Serial.println("No more addresses.");
    Serial.println();
    ds.reset_search();
    delay(250);
    return;
  }
  
  Serial.print("ROM =");
  for( i = 0; i < 8; i++) {
    Serial.write(' ');
    Serial.print(addr[i], HEX);
  }

  if (OneWire::crc8(addr, 7) != addr[7]) {
      Serial.println("CRC is not valid!");
      return;
  }
  Serial.println();
 
  // the first ROM byte indicates which chip
  switch (addr[0]) {
    case 0x10:
      Serial.println("  Chip = DS18S20");  // or old DS1820
      type_s = 1;
      break;
    case 0x28:
      Serial.println("  Chip = DS18B20");
      type_s = 0;
      break;
    case 0x22:
      Serial.println("  Chip = DS1822");
      type_s = 0;
      break;
    default:
      Serial.println("Device is not a DS18x20 family device.");
      return;
  } 

  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1);        // start conversion, with parasite power on at the end
  
  delay(4000);     // maybe 750ms is enough, maybe not
  // we might do a ds.depower() here, but the reset will take care of it.
  
  present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE);         // Read Scratchpad

  Serial.print("  Data = ");
  Serial.print(present, HEX);
  Serial.print(" ");
  for ( i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = ds.read();
    Serial.print(data[i], HEX);
    Serial.print(" ");
  }
  Serial.print(" CRC=");
  Serial.print(OneWire::crc8(data, 8), HEX);
  Serial.println();

  // Convert the data to actual temperature
  // because the result is a 16 bit signed integer, it should
  // be stored to an "int16_t" type, which is always 16 bits
  // even when compiled on a 32 bit processor.
  int16_t raw = (data[1] << 8) | data[0];
  if (type_s) {
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10) {
      // "count remain" gives full 12 bit resolution
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  } else {
    byte cfg = (data[4] & 0x60);
    // at lower res, the low bits are undefined, so let's zero them
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
    //// default is 12 bit resolution, 750 ms conversion time
  }
  celsius = (float)raw / 16.0;
  fahrenheit = celsius * 1.8 + 32.0;
  Serial.print("  Temperature = ");
  Serial.print(celsius);
  Serial.print(" Celsius, ");
  Serial.print(fahrenheit);
  Serial.println(" Fahrenheit");

  //postRequest(messageBuilder(_appKey, _netKey, DS18B20TEMP, HOSTNAME, temperature), http_collector_server_remote);
postRequest(messageBuilder(_appKey, _netKey, DS18B20TEMP, HOSTNAME, fahrenheit ), http_collector_server_remote);

}

int sht30GetMeasurements(int loopCnt){
    float t = sht31.readTemperature();
    float h = sht31.readHumidity();

    if (! isnan(t)) {  // check if 'is not a number'
    Serial.print("Temp *C = "); Serial.print(t); Serial.print("\t\t");
    postRequest(messageBuilder(_appKey, _netKey, SHT30TEMP, HOSTNAME, t), http_collector_server_remote);
    } else { 
    Serial.println("Failed to read temperature");
    }

    if (! isnan(h)) {  // check if 'is not a number'
    Serial.print("Hum. % = "); Serial.println(h);
    postRequest(messageBuilder(_appKey, _netKey, SHT30HUM, HOSTNAME, h), http_collector_server_remote);
    } else { 
    Serial.println("Failed to read humidity");
    }

    delay(1000); //ADJUST THIS TO BE IN SYNC WITH THE ORIGINAL IMPLEMENTATION

    // Toggle heater enabled state every 30 seconds
    // An ~3.0 degC temperature increase can be noted when heater is enabled
    if (loopCnt >= 30) {
    enableHeater = !enableHeater;
    sht31.heater(enableHeater);
    Serial.print("Heater Enabled State: ");
    if (sht31.isHeaterEnabled())
        Serial.println("ENABLED");
    else
        Serial.println("DISABLED");

    loopCnt = 0;
    }
    return loopCnt;
}


PubSubClient client(espClient);

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(2000);
    Serial.print(".");
  }
  WiFi.hostname(HOSTNAME);
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
  randomSeed(micros());
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  // Serial.printf("HostName:%s\n", WiFi.hostname().c_str());
  Serial.println(WiFi.localIP());
}

String messageBuilder(String appKey, String netKey, String deviceId, String _hostname, float value){
  String buf;
    buf += "{\"AppKey\":\"";
    buf += appKey;
    buf += "\",\"NetKey\":\"";
    buf += netKey;
    buf += "\",\"DeviceId\":\"";
    buf += deviceId;
    buf += "\",\"Hostname\":\"";
    buf += String(_hostname);
    buf += "\",\"Value\":\"";
    buf += String(value, 2);
    buf += "\"}";

    Serial.println(buf);

    return buf;
}


void postRequest(String msg, String server)
{
  HTTPClient http;

  if (http.begin(espClient, server)) //Iniciar conexión
  {
    Serial.print("[HTTP] POST...\n");
    http.addHeader("Content-Type", "application/json");
    int httpCode = http.POST(msg);  // Realizar petición

    if (httpCode > 0) {
      Serial.printf("[HTTP] POST... code: %d\n", httpCode);

      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
        String payload = http.getString();   // Obtener respuesta
        Serial.println(payload);
      }
    }
    else {
      Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
  }
  else {
    Serial.printf("[HTTP} Unable to connect\n");
  }

}


void callback(char* topic, byte* payload, unsigned int length) 
{

  String pay_load = "";
  for( int i = 0; i < length; i++ ){
    pay_load = pay_load + (char)payload[i];
    }  
  Serial.println(pay_load); // HERE IS WHERE THE MENSSAGE FROM MQTT ARRIVE
}


void reconnect() 
{
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = HOSTNAME;
    // clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      
      client.publish("forest", HOSTNAME);
      // ... and resubscribe
      //client.subscribe("forest"); // THIS IS WHERE YOU SETUP TE TOPIC
      
      } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup(){
    Serial.begin(9600);
   // dht.setup(D3, DHTesp::DHT22);
   dht.begin();
    setup_wifi();
    client.setKeepAlive(10);
    client.setBufferSize(8192);
    client.setServer(mqtt_server_remote,1883);
    if (!client.connected()) {
        reconnect();
    }
    sensors.begin();
    Wire.begin();

     while (!Serial)
    delay(10);     // will pause Zero, Leonardo, etc until serial console opens

    Serial.println("SHT31 test");
    if (! sht31.begin(0x44)) {   // Set to 0x45 for alternate i2c addr
        Serial.println("Couldn't find SHT31");
        while (1) delay(1);
    }

    Serial.print("Heater Enabled State: ");
    if (sht31.isHeaterEnabled())
        Serial.println("ENABLED");
    else
        Serial.println("DISABLED");
}

void loop()
{   
  long limit = millis() % period;
  if( limit == 0 ) {
    
    reconnect();
     Serial.println("  ");
     Serial.println("  ");
    Serial.println("DHT22");
    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();
    Serial.print("DHT humidity = ");
    Serial.println(humidity, 2);
    Serial.print("DHT temperature C=");
    Serial.println(temperature, 2);
    postRequest(messageBuilder(_appKey, _netKey, DHT22TEMP, HOSTNAME, temperature), http_collector_server_remote);
    postRequest(messageBuilder(_appKey, _netKey, DHT22HUM, HOSTNAME, humidity), http_collector_server_remote);
    delay(5000);

    Serial.println("  ");
    Serial.println("  ");
    Serial.println("ds1820");
    ds1820getMeasurements(); // THIS WILL DO THE POST TO SF DATABASE 

    Serial.println("  ");
    Serial.println("  ");
    Serial.println("SHT30");
    loopCnt++;
    loopCnt = sht30GetMeasurements(loopCnt); //YOU NEED TO ADJUST THE DELAY AND THE LOOP COUNT BECAUSE INSIDE THIS FUNCTION THERE IS A DELAY TOO IN LINE 169

    delay(5000);
    
    Serial.println("  ");
    Serial.println("  ");
    Serial.println("DS18B20");
    sensors.requestTemperatures(); 
    float temperatureC = sensors.getTempCByIndex(0);
    Serial.print(temperatureC);
    postRequest(messageBuilder(_appKey, _netKey, DS18B20TEMP, HOSTNAME, temperatureC), http_collector_server_remote);
    delay(5000);

  Serial.println("  ");
  Serial.println("  ");
  Serial.println("Battery VOLTAGE sensor / A13"); 
  int value = analogRead(35);
  Serial.println(value);
  // original voltage equation 
  //vOUT=((value * 3.3) / 1024.0);
  // vIN = vOUT / (R2/(R1+R2)); /// what the hell are you doing here?
  float voltageLevel = (value / 4095.0) * 2 * 1.1 * 3.3; // calculate voltage level
  // float voltageLevel = (rawValue / 4095.0) * 1.18 * 3.3; // calculate voltage level
  float batteryFraction = voltageLevel / MAX_BATTERY_VOLTAGE;
  int vOUT= voltageLevel;
  int vIN = voltageLevel;
  Serial.println((String)"Raw:" + value + " Voltage:" + voltageLevel + "V Percent: " + (batteryFraction * 100) + "%");
  Serial.println(vIN);
    postRequest(messageBuilder(_appKey, _netKey, VOLTAGE, HOSTNAME, vIN), http_collector_server_remote);
    delay(5000);

 }

}
