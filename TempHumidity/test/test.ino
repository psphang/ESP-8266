#include <dummy.h>

#include <ESP8266WiFi.h>
#include <WiFiClient.h>

#define USERNAME "psphang"
#define DEVICE_ID ESP.getChipId()
#define DEVICE_CREDENTIAL "12345678"

const char* ssid     = "streamxy_hotspot";
const char* password = "29012010";

int    temp_max = 1023;
float  temp_mult = 1;
int    hum_max = 1023;
float  hum_mult = 1;

WiFiClient client;
byte server[] = { 
  192, 168, 2, 3 }; // http server - PUT HERE YOUR SERVER IP as bytes
String serverStr = "192.168.2.3"; // http server - PUT HERE YOUR SERVER IP as string

float humidity, temperature;  // Values read from sensor
String webString = "";   // String to display
// Generally, you should use "unsigned long" for variables that hold time
unsigned long previousMillis = 0;        // will store last temp was read
const long interval = 2000;              // interval at which to read sensor



//Temperature sensor
// resistance at 25 degrees C
#define THERMISTORNOMINAL 50000      
// temp. for nominal resistance (almost always 25 C)
#define TEMPERATURENOMINAL 25  
// The beta coefficient of the thermistor (usually 3000-4000)
#define BCOEFFICIENT 4000
// the value of the 'other' resistor
#define R1 51000 
#define R2 51000 
#define R3 51000 
#define Vs 3.20 //Offset 0,1 of pin out low <> Gnd

//Humidity sensor
//voltage 1 output and Humidity 1 =40% at temprature 25 C 
#define H1 40
#define V1 1.282
//voltage 2 output and Humidity 2 =60% at temprature 25 C 
#define H2 60
#define V2 1.570



// how many samples to take and average, more takes longer
// but is more 'smooth'
#define NUMSAMPLES 10
 
int samples[NUMSAMPLES];
int acc,ave,readval;
void setup() {
  Serial.begin(115200);  // Serial connection from ESP-01 via 3.3v console cable
  Serial.println("\n\r \n\rWorking to connect");
  for (uint8_t t = 4; t > 0; t--) {
    Serial.print("[SETUP] WAIT ");
    Serial.println(t);
    delay(1000);
  }
 //get_temp();
// get_hum();
//gettemperature();
//  Serial.println(String(temperature));
 
 // Connect to WiFi network
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("\n\r \n\rWorking to connect");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
    Serial.print(".");
  }
  Serial.println("");
  delay(6000);
  

}
void loop(void)
{
  delay(2);
 
  // get_temp();
   Serial.print("Temperature :");
   Serial.println(temperature); 
  // get_hum();
   Serial.print("Humidity :");
   Serial.println(humidity);  
 
 
//  Serial.println("DHT Weather Reading Client");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println(DEVICE_ID);
//  Serial.println(ESP.getChipId()) ;
 Serial.println("Client started");
 delay(1000);

// IPAddress ip(192,168,2,3);
// client.connect(ip, 80);
// delay(1000);
// if (client.connected()) {
 if (client.connect(server, 80)) {  // http server is running on default port 80
   Serial.println("connected");
    client.print("GET /temphum/tempdata.aspx?temp=");  // PUT HERE YOUR SERVER URL e.g. from http://192.168.1.11/SensorWriteToFile.php?temp=10.5&hum=58

    client.print(String(temperature));
    client.print("&hum=");
    client.print(String(humidity));
    client.print("&SID=");
    client.print(String(DEVICE_ID));
    client.println(" HTTP/1.1");
    client.print("Host: ");   // http server is running on port 80, so no port is specified
    client.println(serverStr); 
    client.println("User-Agent: Mihi IoT 01");   // PUT HERE YOUR USER-AGENT that can be used in your php program or Apache configuration
   // client.println(); // empty line for apache server

    //Wait up to 10 seconds for server to respond then read response
    int i = 0;
    while ((!client.available()) && (i < 1000)) {
      delay(10);
      i++;
    }

    while (client.available())
    {
      String Line = client.readStringUntil('\r');
      Serial.print(Line);
    }
    client.stop();
  } 
  else {
    Serial.println("connection failed");
  }
  Serial.println();
  Serial.println(WiFi.status());

  //WiFi.disconnect(); // DO NOT DISCONNECT WIFI IF YOU WANT TO LOWER YOUR POWER DURING LIGHT_SLEEP_T DELLAY !
  Serial.println(WiFi.status());  

  //wifi_set_sleep_type(LIGHT_SLEEP_T);

  //delay(60000*3-800); // loop every 3 minutes

}




static unsigned int get_temp()
{uint8_t i;
  float average,V3,T1;
  
  pinMode(16,INPUT);
  pinMode(14,OUTPUT);
  digitalWrite(14,LOW);
  delay(2);
 // return (unsigned int)max(0, min(1023,((temp_max-analogRead(A0))*temp_mult)));
 
 
  // take N samples in a row, with a slight delay
  for (i=0; i< NUMSAMPLES; i++) {
   samples[i] = analogRead(A0);
   delay(100);
  }
 
  // average all the samples out
  average = 0;
  for (i=0; i< NUMSAMPLES; i++) {
     average += samples[i];
  }
  average /= NUMSAMPLES;
  V3=average/1024*1;
//  Serial.print("Vs : "); 
//  Serial.println(Vs); 
//  Serial.print("V3 : "); 
//  Serial.println(V3); 
//Find Resistance by ADC (voltage)
T1=-((R1*V3*(R2+R3))/(V3*(R1+R2)+R3*(V3-Vs)));

// Convering resistance to temperature 
float steinhart;
  steinhart = T1/ THERMISTORNOMINAL;     // (R/Ro)
  steinhart = log(steinhart);                  // ln(R/Ro)
  steinhart /= BCOEFFICIENT;                   // 1/B * ln(R/Ro)
  steinhart += 1.0 / (TEMPERATURENOMINAL + 273.15); // + (1/To)
  steinhart = 1.0 / steinhart;                 // Invert
  steinhart -= 273.15;                         // convert to C

   temperature=steinhart;
}

static unsigned int get_hum()
{
  uint8_t i;
  float average,Vadc,Vh,m;
  
  pinMode(14,INPUT);
  pinMode(16,OUTPUT);
  digitalWrite(16,LOW);
  delay(2);

   // take N samples in a row, with a slight delay
   ave=0;
  for (i=0; i< NUMSAMPLES; i++) {
    samples[i] = analogRead(A0);
    readval=samples[i];
    if (i>0) {
      if (abs(ave-readval)>3 ) {
        i=0;
        ave=0;
        acc=0;
      }
    
    }
    acc=acc+readval;
    ave=acc/(i+1);
   delay(100);
  }
 
  // average all the samples out
  average = 0;
  for (i=0; i< NUMSAMPLES; i++) {  
     average += samples[i];
  }
  average /= NUMSAMPLES;
  Vadc=average/1024*1;
  Vh=Vadc/R3*(R1+R2+R3);
  Vh=Vh+0.04;//offset //Offset 0,1 of pin out low <> Gnd
//Slope 
 m =(V2-V1)/(H2-H1);
  
//  Serial.print("Vadc : "); 
//  Serial.println(Vadc); 
//  Serial.print("Vh : "); 
//  Serial.println(Vh); 
//
//  Serial.print("m : "); 
//  Serial.println(m); 
//  Serial.print("H1 : "); 
//  Serial.println(H1); 
//  Serial.print("V1 : "); 
//  Serial.println(V1); 
 // return (unsigned int)max(0, min(1023,((hum_max-analogRead(A0))*hum_mult)));
 humidity=(H1*m+Vh-V1)/m;
}





