#include <SoftwareSerial.h>
//#include "Statistic.h"
#include <WiFiClient.h>
#include <ESP8266WiFiMulti.h>
ESP8266WiFiMulti wifiMulti;
#define DEVICE_ID ESP.getChipId()

WiFiClient client;
byte server[] = {10, 236, 80, 189 }; // http server - PUT HERE YOUR SERVER IP as bytes
String serverStr = "10.236.80.189"; // http server - PUT HERE YOUR SERVER IP as string
//Variable for quearysting and default value
int netsts = 0;
int datWrt, sts, cov5, cov13;



//Statistic myStats;

// set up a new serial object
SoftwareSerial mySerial(D2, D1); //(rx, tx)

char byteRead;

//infared sensor
int irVal = 0;
int irPin = D5;

//serial processing
char inData[50]; // Allocate some space for the string
char inChar=-1; // Where to store the character read


String rslt;
float volt;

int five_pass = 0;
int trdt_pass = 0;

//reading define
const float mid = 4;
const float upbndry = 9.0;
const float lowbndry = 1;
const float hi_lmt = 6;
const float lo_lmt = 2.19;

//led control
const int blueled_pass = D3;
const int redled_ng = D4;
const int yellowled_guide = D0;


int stable;

int uploaded = 0;

void off_led(){
    digitalWrite(blueled_pass, 1);//blue led Off
    digitalWrite(redled_ng, 1); //red led Off
    digitalWrite(yellowled_guide, 1); //yellow led Off
}
void pass_led() {
  off_led();
  digitalWrite(blueled_pass, 0);//blue led On
}
void ng_led() {
  off_led();
  digitalWrite(redled_ng, 0); //red led On 
}
void guide_led() {
  off_led();
  digitalWrite(yellowled_guide, 0); //yellow led on
}
void error_led(){
  digitalWrite(blueled_pass, 0);//blue led On
  digitalWrite(redled_ng, 0); //red led On
  digitalWrite(yellowled_guide, 0); //yellow led On
}


void read_com() {
    byte inde =0;                       // Index into array; where to store the character
   
    while (mySerial.available() > 0)    // Don't read unless there you know there is data
    {
        if(inde < 49)                   // One less than the size of the array
        {
            inChar = mySerial.read();    // Read a character
            inData[inde] = inChar; // Store it
            inde++; // Increment where to write next
            inData[inde] = '\0'; // Null terminate the string 
            //delay(100);
            rslt = inData;
            String val = rslt.substring(0,6); 
            String expn = rslt.substring(7);
            float val2=val.toFloat();
            float expn2=expn.toFloat();
            double base = 10;
            float power= pow(base, expn2);
            volt = val2*power;
           }
    }

}

void get_volt(){
      mySerial.print("VAL?");
      mySerial.print("\r\n");
      delay(50);
  read_com();
  while (volt < 1 || volt >=10 || isnan(volt) || isinf(volt)){
      delay(200);
      mySerial.print("VAL?");
      mySerial.print("\r\n");
      delay(50);
    read_com();
  }
}

void setup(){
  Serial.begin(19200); 
  mySerial.begin(9600);

  pinMode(blueled_pass, OUTPUT);
  pinMode(redled_ng, OUTPUT);
  pinMode(yellowled_guide, OUTPUT);
  pinMode(irPin, INPUT_PULLUP);

  error_led();
  //myStats.clear();


  mySerial.print("RATE F"); 
  mySerial.print("\r\n");
  delay(100);
  read_com();
  mySerial.print("VDC");
  mySerial.print("\r\n");
  delay(100);
  read_com();
  mySerial.print("RANGE 3");
  mySerial.print("\r\n");
  delay(100);
  read_com();
  mySerial.print("format 1");
  mySerial.print("\r\n");
  delay(100);
  read_com();
  mySerial.print("REMS");
  mySerial.print("\r\n");
  delay(100);
  read_com(); 
  delay(100);
  read_com();

  off_led();

  Serial.println("");
  Serial.print("Connecting to WIFI: ");
  pass_led();

wifiMulti.addAP("L80173RTD01", "@Lanetwork07456541");
wifiMulti.addAP("L80193RTD02", "@Lanetwork36205257");


if(wifiMulti.run() == WL_CONNECTED) {
        Serial.println("");
        Serial.print("WiFi connected :  ");
        Serial.print(WiFi.SSID());
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());
  }
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    wifiMulti.run();
  }
  netsts = 1;  
  off_led();
}

 

void upload(){
    if (client.connect(server, 81)) {  // http server is running on default port 80
      Serial.print("connected Web Server IP: ");
      Serial.println(serverStr);
      datWrt = 1;  
      String method = "GET /photointerupter/voltdata.aspx";  // PUT HERE YOUR SERVER URL e.g. from http://192.168.1.11/SensorWriteToFile.php?temp=10.5&hum=58
             method +="?SID=" + String(DEVICE_ID);
             method +="&volt=" + String(volt); 
             method +="&netsts=" + String(netsts) ;
             method += " HTTP/1.1\r\n ";
             method += "Host: " + serverStr +"\r\n";   // http server is running on port 80, so no port is specified
             method += "User-Agent: Mihi IoT 01\r\n";  // PUT HERE YOUR USER-AGENT that can be used in your php program or Apache configuration
             method += "\r\n";
             method += "\r\n";
             Serial.println(method);
         
      client.print(method);  
    
      int i = 0;
      while ((!client.available()) && (i < 1000)) {         //Wait up to 10 seconds for server to respond then read response
        delay(10);
        i++;
        }  
      while (client.available()){
        String Line = client.readStringUntil('\r');
        Serial.println(Line);
        //readtags( Line); //process readtags from HTML page 
        }
      netsts=0; //reset netsts after send to server
      client.stop();
      } 
      else {
        Serial.println("Web Server connection failed");
        error_led();  //red led on (Web server connection fail)
      }
 }
 

void readtags(String content){
  char *starttags[] = { "<sts>", "<cov5>", "<cov13>","<datWrt>" };
  char *endtags[] = { "</sts>", "</cov5>", "</cov13>","</datWrt>" };
  String tagdata[5];
  for (int i=0; i<4; i++ )
  {   
   int startValue = content.indexOf(starttags[i]);
      // Serial.println(startValue);
   int endValue = content.indexOf(endtags[i]);
      //   Serial.println(endValue);
      // Serial.println(starttags[i]);
   int k = strlen(starttags[i]);
   tagdata[i] = content.substring(startValue + k, endValue);    // + 7 to make the index start after the <Value> tag  
    Serial.println(tagdata[i]); 
   }
      sts=tagdata[0].toInt();
      cov5 =tagdata[1].toInt();
      cov13=tagdata[2].toInt();
      datWrt=tagdata[3].toInt();

}

void loop(){
 
  //Serial.print("Device IP address: ");
  //Serial.println(WiFi.SSID());
  //Serial.println(WiFi.localIP());
  //Serial.println(DEVICE_ID);
  //Serial.println("Client started");
 
  if(wifiMulti.run() != WL_CONNECTED){                //reconnect WiFi
    Serial.println("WiFi not connected!");
    delay(1000);
    netsts = 2;
  }
  
    irVal = digitalRead(irPin);
    digitalRead(irPin);
    off_led();
  if(irVal == 1 && uploaded == 1){
    delay(150);
    uploaded = 0;
    digitalRead(irPin);
  }

  if(irVal == 0 && uploaded == 0){
    delay(500);
    get_volt();
    Serial.println(volt);
    uploaded = 1;
    pass_led();
    upload();
  }
}


   
