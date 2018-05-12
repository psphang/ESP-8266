#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include "DHT.h"
#include "DHT_U.h"
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

#include <WiFiClient.h>
#include <ESP8266WiFiMulti.h>
ESP8266WiFiMulti wifiMulti;
#define DEVICE_ID ESP.getChipId()

WiFiClient client;
byte server[] = {10, 236, 80, 189 }; // http server - PUT HERE YOUR SERVER IP as bytes
String serverStr = "10.236.80.189"; // http server - PUT HERE YOUR SERVER IP as string
//byte server[] = {192,168,2,30 }; // http server - PUT HERE YOUR SERVER IP as bytes
//String serverStr = "192.168.2.30"; // http server - PUT HERE YOUR SERVER IP as string
//Variable for quearysting and default value
int sts, dspIntv = 10, recIntv = 1200, datWrt;


// DHT Sensor
const int DHTPin = D5; //or GPIO14
// Initialize DHT sensor.
DHT dht(DHTPin, DHTTYPE);
float humidity, temperature;  // Values read from sensor
String dsprec = "dsp";
int rec_cnt = 0;
int netsts = 0;

// Temporary variables
static char celsiusTemp[7];
static char fahrenheitTemp[7];
static char humidityTemp[7];

//LED indicator
const int blueled_sts = D6;
const int redled_fail = D7;
volatile int toggle_sts, toggle_fail;
int led_sts;
int led_fail;
void inline handler (void) {
  LEDctrl();

  //timer0_write(ESP.getCycleCount() + 41660000);
  timer0_write(ESP.getCycleCount() + 80000000L); // 80MHz == 1sec
}
void LEDctrl() {
  if (led_sts == 2) {
    toggle_sts = (toggle_sts == 1) ? 0 : 1;      //blink
  } else if (led_sts == 1) {
    toggle_sts = 0; //on
  } else {
    toggle_sts = 1;  //off
  }
  digitalWrite(blueled_sts, toggle_sts);


  if (led_fail == 2) {
    toggle_fail = (toggle_fail == 1) ? 0 : 1;     //blink
  } else if (led_fail == 1) {
    toggle_fail = 0; //on
  } else {
    toggle_fail = 1;  //off
  }
  digitalWrite(redled_fail, toggle_fail);
}



void setup() {
  // Initializing serial port for debugging purposes
  Serial.begin(115200);
  delay(10);

  //LED indicator
  pinMode(blueled_sts, OUTPUT);
  pinMode(redled_fail, OUTPUT);
  //  noInterrupts();
  //  timer0_isr_init();
  //  timer0_attachInterrupt(handler);
  // // timer0_write(ESP.getCycleCount() + 41660000);
  //  timer0_write(ESP.getCycleCount() + 80000000L); // 80MHz == 1sec
  // interrupts();


  Serial.println("\n\r \n\rWorking to connect");
  for (uint8_t t = 4; t > 0; t--) {
    Serial.print("[LOANDING] WAIT ");
    Serial.println(t);
    delay(1000);
    digitalWrite(blueled_sts, 0);//blue led On
    digitalWrite(redled_fail, 0); //red led On
    netsts = 1; //network connection start up
  }
  digitalWrite(redled_fail, 1); //red led Off
  // We start by connecting to a WiFi network
  Serial.println("");
  Serial.print("Connecting to WIFI: ");

  wifiMulti.addAP("L80173RTD01", "@Lanetwork07456541");
  wifiMulti.addAP("L80193RTD02", "@Lanetwork36205257");
  wifiMulti.addAP("mem002", "Mem002P@ssword");


  if (wifiMulti.run() == WL_CONNECTED) {
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  }

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    wifiMulti.run();
    digitalWrite(blueled_sts, 0);//blue led On
  }

  led_sts = 2 ; //blue led blinking

  dht.begin();




}

void loop() {

  // Connect to WiFi network
  //  WiFi.mode(WIFI_STA);
  //  WiFi.begin(ssid, password);
  //  Serial.print("\n\r \n\rWorking to connect");

  // Wait for connection(retry due to connection fail)
  while (wifiMulti.run() != WL_CONNECTED) {
    delay(200);
    Serial.print(".");
    digitalWrite(blueled_sts, 0);//blue led On
    digitalWrite(redled_fail, 1); //red led off
    netsts = 2 ;

  }



  String ssid = WiFi.SSID();
  Serial.println("");
  Serial.println("DHT Weather Reading Client");
  Serial.print("Connected to WIFI: ");
  Serial.println(ssid);
  Serial.print("Device IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println(DEVICE_ID);
  //  Serial.println(ESP.getChipId()) ;
  Serial.println("Client started");

  //Get Temperature and humidity data
  while (!get_DHT()) {
    Serial.println("Retry read DHT");
    delay(1000);
    digitalWrite(blueled_sts, 0);//blue led On

  }
  led_sts = 2; //blue led blinking

  Serial.print("Temperature = ");
  Serial.println(temperature);
  Serial.print("Humidity = ");
  Serial.println(humidity);



  //delay timer for record to database
  rec_cnt = rec_cnt + dspIntv;
  Serial.print("rec_cnt=");
  Serial.println(rec_cnt);
  if (rec_cnt >= recIntv) {
    dsprec = "rec";
    rec_cnt = 0;
  } else {
    dsprec = "dsp";
  }
  Serial.println(dsprec);

  if (client.connect(server, 81)) {  // http server is running on default port 80
    Serial.print("connected Web Server IP: ");
    Serial.println(serverStr);

    if (!sts && !datWrt) { // If Not Temp/Hum out off range
      led_fail = 0; //red led off
    } else {
      if (sts)
        led_fail = 2; //red led blink (temp/Hum out of range)
      led_sts = 0; //blue led OFF
      if (datWrt)
        led_fail = 1; //red led ON (Data write fail )
    }
    datWrt = 1; //flag data write to SQL result
    String method = "GET /TempHum/Tempdata.aspx";  // PUT HERE YOUR SERVER URL e.g. from http://192.168.1.11/SensorWriteToFile.php?temp=10.5&hum=58
    method += "?SID=" + String(DEVICE_ID);
    method += "&Temperature=" + String(temperature);
    method += "&Humidity=" + String(humidity);
    method += "&SSID=" + ssid;
    method += "&dsprec=" + dsprec ;
    method += "&netsts=" + String(netsts) ;
    method += " HTTP/1.1\r\n ";
    method += "Host: " + serverStr + "\r\n";  // http server is running on port 80, so no port is specified
    method += "User-Agent: Mihi IoT 01\r\n";  // PUT HERE YOUR USER-AGENT that can be used in your php program or Apache configuration
    method += "\r\n";
    method += "\r\n";
    // Serial.println(method);

    client.print(method);

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
      readtags( Line); //process readtags from HTML page
    }
    netsts = 0; //reset netsts after send to server
    client.stop();
  }
  else {
    Serial.println("Web Server connection failed");
    led_fail = 1; //red led on (Web server connection fail)
  }
  Serial.println();
  //Serial.println(WiFi.status());

  //WiFi.disconnect(); // DO NOT DISCONNECT WIFI IF YOU WANT TO LOWER YOUR POWER DURING LIGHT_SLEEP_T DELLAY !
  //Serial.println(WiFi.status());

  //wifi_set_sleep_type(LIGHT_SLEEP_T);
  Serial.println(sts);
  Serial.println(dspIntv);
  Serial.println(recIntv);
  Serial.println(datWrt);
  int i;
  for (int i = 0; i < dspIntv; i++) {
    LEDctrl();
    delay(864); //1 Secc - above process time
  }
  //delay(60000*3-800); // loop every 3 minutes


}

void readtags(String content)
{
  char *starttags[] = { "<sts>", "<dspIntv>", "<recIntv>", "<datWrt>" };
  char *endtags[] = { "</sts>", "</dspIntv>", "</recIntv>", "</datWrt>" };
  String tagdata[5];


  for (int i = 0; i < 4; i++ )
  { // Serial.println(i);
    // delay(1000);
    // Find the <Value> and </Value> tags and make a substring of the content between the two
    int startValue = content.indexOf(starttags[i]);
    // Serial.println(startValue);
    int endValue = content.indexOf(endtags[i]);
    //   Serial.println(endValue);
    // Serial.println(starttags[i]);
    int k = strlen(starttags[i]);

    tagdata[i] = content.substring(startValue + k, endValue);    // + 7 to make the index start after the <Value> tag


    //   Serial.println(tagdata[i]);

  }
  sts = tagdata[0].toInt();
  dspIntv = tagdata[1].toInt();
  recIntv = tagdata[2].toInt();
  datWrt = tagdata[3].toInt();
  //  Serial.println(sts);
  //  Serial.println(dspIntv);
  //  Serial.println(recInt);
  //  Serial.println(datWrt);
}




static unsigned int get_DHT()
{
  humidity = 0;
  temperature = 0;
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);
  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Failed to read from DHT sensor!");
    strcpy(celsiusTemp, "Failed");
    strcpy(fahrenheitTemp, "Failed");
    strcpy(humidityTemp, "Failed");
    return (0);
  } else {
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

    humidity = h;
    temperature = t;


    return (1);
  }
}


