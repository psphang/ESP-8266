

#include <ESP8266WiFi.h>
#include "DHT.h"
#include "DHT_U.h"
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
#include <ESP8266WiFi.h>

const char* ssid = "steamyxr_hotspot";
const char* password = "29012010";
IPAddress host(192,168,4,1);

// DHT Sensor
const int DHTPin = D5; //or GPIO14 
// Initialize DHT sensor.
DHT dht(DHTPin, DHTTYPE);

// Temporary variables
static char celsiusTemp[7];
static char fahrenheitTemp[7];
static char humidityTemp[7];

void setup() {
  // Initializing serial port for debugging purposes
  Serial.begin(115200);
  delay(10);

  dht.begin();

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
  
}

void loop() {
  
  Serial.print("connecting to ");
  Serial.println(host);
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }




  
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
        Serial.println(" *F");
        
        // We now create a URI for the request
        String url = "/input/";
        url += streamId;
        url += "?private_key=";
        url += privateKey;
        url += "&value=";
        url += value;
        
        Serial.print("Requesting URL: ");
        Serial.println(url);
       
        // This will send the request to the server
        client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                     "Host: " + host + "\r\n" + 
                     "Connection: close\r\n\r\n");
        unsigned long timeout = millis();
        while (client.available() == 0) {
          if (millis() - timeout > 5000) {
            Serial.println(">>> Client Timeout !");
            client.stop();
            return;
          }
          // Read all the lines of the reply from server and print them to Serial
        while(client.available()){
          String line = client.readStringUntil('\r');
          Serial.print(line);
        }
  
        
     }

}
