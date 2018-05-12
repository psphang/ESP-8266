#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include "DHT.h"
#include "DHT_U.h"
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

#include <WiFiClient.h>

#define DEVICE_ID ESP.getChipId()
//const char* ssid     = "mem001";
//const char* password = "Mem001password";
const char* ssid     = "streamxy_hotspot";
const char* password = "29012010";


WiFiClient client;
//byte server[] = {10, 236, 80, 189 }; // http server - PUT HERE YOUR SERVER IP as bytes
//String serverStr = "10.236.80.189"; // http server - PUT HERE YOUR SERVER IP as string
byte server[] = {192,168,2,30 }; // http server - PUT HERE YOUR SERVER IP as bytes
String serverStr = "192.168.2.30"; // http server - PUT HERE YOUR SERVER IP as string
//Send Request interval and return status from web sever
int dsp=15 ; //15 Sec 
int rsc=1200;  //20 minit
String sts;

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
  
  Serial.println("\n\r \n\rWorking to connect"); 
   for (uint8_t t = 4; t > 0; t--) { 
     Serial.print("[LOANDING] WAIT "); 
     Serial.println(t); 
    delay(1000); 
   } 

  // We start by connecting to a WiFi network
  Serial.println("");
  Serial.print("Connecting to WIFI: ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
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



while (!get_DHT()) {
  Serial.println("Retry read DHT"); 
  delay(1000);
}
Serial.print("Temperature = ");
Serial.println(temperature);
Serial.print("Humidity = "); 
Serial.println(humidity); 
   
  // Connect to WiFi network
//  WiFi.mode(WIFI_STA);
//  WiFi.begin(ssid, password);
//  Serial.print("\n\r \n\rWorking to connect");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("DHT Weather Reading Client");
  Serial.print("Connected to WIFI: ");
  Serial.println(ssid);
  Serial.print("Device IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println(DEVICE_ID);
//  Serial.println(ESP.getChipId()) ;
 Serial.println("Client started");

  if (client.connect(server, 80)) {  // http server is running on default port 80
    Serial.print("connected Web Server IP: ");
    Serial.println(serverStr);  
    client.print("GET /TempHum/Tempdata.aspx?SID=");  // PUT HERE YOUR SERVER URL e.g. from http://192.168.1.11/SensorWriteToFile.php?temp=10.5&hum=58
    client.print(String(DEVICE_ID));
    client.print("&Temperature=");
    client.print(String(temperature));
    client.print("&Humidity=");
    client.print(String(humidity));
    client.print("&dsp_rec=");
    client.print("rec");
    client.println(" HTTP/1.1");
    client.print("Host: ");   // http server is running on port 80, so no port is specified
    client.println(serverStr); 
    client.println("User-Agent: Mihi IoT 01");   // PUT HERE YOUR USER-AGENT that can be used in your php program or Apache configuration
    client.println(); // empty line for apache server
//client.print( "Connection: close\r\n\r\n");
    //Wait up to 10 seconds for server to respond then read response
    int i = 0;
    while ((!client.available()) && (i < 1000)) {
      delay(10);
      i++;
    }

    while (client.available())
    {
      String Line = client.readStringUntil('\r');
      Serial.println(Line);
      
    }
    client.stop();
  } 
  else {
    Serial.println("Web Server connection failed");
  }
  Serial.println();
  Serial.println(WiFi.status());

  //WiFi.disconnect(); // DO NOT DISCONNECT WIFI IF YOU WANT TO LOWER YOUR POWER DURING LIGHT_SLEEP_T DELLAY !
  Serial.println(WiFi.status());  

  //wifi_set_sleep_type(LIGHT_SLEEP_T);
delay(5000);
  //delay(60000*3-800); // loop every 3 minutes






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
      return(0);    
     }else{
        // Computes temperature values in Celsius + Fahrenheit and Humidity
        float hic = dht.computeHeatIndex(t, h, false);       
        dtostrf(hic, 6, 2, celsiusTemp);             
        float hif = dht.computeHeatIndex(f, h);
        dtostrf(hif, 6, 2, fahrenheitTemp);         
        dtostrf(h, 6, 2, humidityTemp);
        //String data = "hum=" + (string)h +"temp=" + (String)t;

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
        

        return(1);
     }
}


