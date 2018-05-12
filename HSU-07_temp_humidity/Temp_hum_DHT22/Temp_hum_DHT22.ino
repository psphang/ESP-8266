

#include <ESP8266WiFi.h>
#include "DHT.h"
#include "DHT_U.h"
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

#include <WiFiClient.h>

#define DEVICE_ID ESP.getChipId()
const char* ssid     = "mem001";
const char* password = "Mem001password";

WiFiClient client;
byte server[] = { 
  10, 236, 80, 189 }; // http server - PUT HERE YOUR SERVER IP as bytes
String serverStr = "10.236.80.189"; // http server - PUT HERE YOUR SERVER IP as string



// DHT Sensor
const int DHTPin = D5; //or GPIO14 
// Initialize DHT sensor.
DHT dht(DHTPin, DHTTYPE);
float humidity, temperature;  // Values read from sensor


// Temporary variables
static char celsiusTemp[7];
static char fahrenheitTemp[7];
static char humidityTemp[7];

void setup() {
  // Initializing serial port for debugging purposes
  Serial.begin(115200);
  delay(10);
 
  // We start by connecting to a WiFi network
  //Serial.println();
  Serial.println("");
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
  dht.begin();

}

void loop() {
sts=getDHT;

Serial.print(temperature);
Serial.println(humidity);  
delay(1000);
}

static unsigned int get_DHT()
{
  humidity =0;
  temperature=0;
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
     }else{
        // Computes temperature values in Celsius + Fahrenheit and Humidity
        float hic = dht.computeHeatIndex(t, h, false);       
        dtostrf(hic, 6, 2, celsiusTemp);             
        float hif = dht.computeHeatIndex(f, h);
        dtostrf(hif, 6, 2, fahrenheitTemp);         
        dtostrf(h, 6, 2, humidityTemp);
        String data = "hum=" + (string)h +"temp=" + (String)t;

        // You can delete the following Serial.print's, it's just for debugging purposes
//        Serial.print("Humidity: ");
//        Serial.print(h);
//        Serial.print(" %\t Temperature: ");
//        Serial.print(t);
//        Serial.print(" *C ");
//        Serial.print(f);
//        Serial.print(" *F\t Heat index: ");
//        Serial.print(hic);
//        Serial.print(" *C ");
//        Serial.print(hif);
//        Serial.println(" *F");
  
        humidity =h;
        temperature=t;
        

        return(0);

}

