#include <SoftwareSerial.h>
//#include "Statistic.h"
#include <WiFiClient.h>
#include <ESP8266WiFiMulti.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Bounce2.h>

ESP8266WiFiMulti wifiMulti;

#define DEVICE_ID ESP.getChipId()

WiFiClient espclient;
byte server[] = {10, 236, 80, 189 }; // http server - PUT HERE YOUR SERVER IP as bytes
String serverStr = "10.236.80.189"; // http server - PUT HERE YOUR SERVER IP as string
//MQTT
const char* mqttServer = "10.236.80.189";
const int mqttPort = 1883;
const char* mqttUser = "owntracks";
const char* mqttPassword = "owntracks";
PubSubClient mqttclient(espclient);
//TOPIC Publish and Subscribe
char subtopic[80];  //subscribe topic buffer concatenate "publictopic" + "DEVICE_ID"
char pubtopic[]="esp/move/";



//Variable for quearysting and default value
int netsts = 0;
int  sts=9 ,opt;
String inout,username,cardno;
unsigned oldtag;

const int BUFFER_SIZE = 14; // RFID DATA FRAME FORMAT: 1byte head (value: 2), 10byte data (2byte version + 8byte tag), 2byte checksum, 1byte tail (value: 3)
const int DATA_SIZE = 10; // 10byte data (2byte version + 8byte tag)
const int DATA_VERSION_SIZE = 2; // 2byte version (actual meaning of these two bytes may vary)
const int DATA_TAG_SIZE = 8; // 8byte tag
const int CHECKSUM_SIZE = 2; // 2byte checksum;
char buffer[BUFFER_SIZE]; // used to store an incoming data frame 
int buffer_index = 0;

// set up a new serial object
SoftwareSerial mySerial(D7, D8); //(rx, tx)

char byteRead;






//serial processing
char inData[50]; // Allocate some space for the string
char inChar=-1; // Where to store the character read


int stop_read_card_cnt=0;


const int out_buzzer = D5;
//const int blueled = D6;
const int blueled = D4;     //LED_BUILDIN; //GPIO02;
const int flash_btnPin=D3; //GPIO16;
Bounce pushbutton = Bounce(flash_btnPin, 100);  // 10 ms debounce

void setup(){
  pinMode(out_buzzer, OUTPUT);//buzzer
  pinMode(blueled, OUTPUT);//led indicator when singing a note
  pinMode(flash_btnPin, INPUT); //GPIO16;

  
  Serial.begin(19200); 
  mySerial.begin(9600);
  mySerial.listen();
  
  Serial.println("");
  Serial.print("Connecting to WIFI: ");
  

wifiMulti.addAP("L80173RTD01", "@Lanetwork07456541");
wifiMulti.addAP("L80193RTD02", "@Lanetwork36205257");
wifiMulti.addAP("mem002", "Mem002P@ssword");

if(wifiMulti.run() == WL_CONNECTED) {
        Serial.println("");
        Serial.print("WiFi connected :  ");
        Serial.print(WiFi.SSID());
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());
} 
  while (WiFi.status() != WL_CONNECTED) {
   // delay(500);
    Serial.print(".");
    wifiMulti.run();
    digitalWrite(blueled, HIGH); //LED OFF
  delay(500);
  digitalWrite(blueled, LOW); //LED ON
  delay(500);
  }
 
  netsts = 1;  
  
 String ssid = WiFi.SSID();
  Serial.println("");
  Serial.print("Connected to WIFI: ");
  Serial.println(ssid);
  Serial.print("Device IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println(DEVICE_ID);
  //  Serial.println(ESP.getChipId()) ;
  Serial.println("Client started");


  mqttclient.setServer(mqttServer, mqttPort);
  mqttclient.setCallback(callback);
 
  while (!mqttclient.connected()) {
    Serial.println("Connecting to MQTT...");
 
    if (mqttclient.connect("ESP32Client", mqttUser, mqttPassword )) {
 
      Serial.println("connected");  
      digitalWrite(blueled, LOW); //LED ON
      //Subcribe TOPIC
      sprintf(subtopic,"%s%i",pubtopic,DEVICE_ID); //subscribe topic= concatenate "publictopic" + "DEVICE_ID"
      mqttclient.subscribe(subtopic);
      Serial.print("Subscibed Topic: ");
      Serial.println(subtopic);


      
    } else {
 
      Serial.print("failed with state ");
      Serial.print(mqttclient.state());
      delay(2000);
 
    }
  }
 

  
  
 
}

 


void loop(){
 // Wait for connection(retry due to connection fail)
  while (wifiMulti.run() != WL_CONNECTED) {
    //delay(200);
    Serial.print(".");
    
    netsts = 2 ;
    
    digitalWrite(blueled, HIGH); //LED OFF
    delay(100);
    digitalWrite(blueled, LOW); //LED ON
    delay(100);
  }
  while (!mqttclient.connected()) {
    Serial.println("Connecting to MQTT...");
 
    if (mqttclient.connect("ESP32Client", mqttUser, mqttPassword )) {
 
      Serial.println("connected");  
      digitalWrite(blueled, LOW); //LED ON
      //Subcribe TOPIC
      sprintf(subtopic,"%s%i",pubtopic,DEVICE_ID); //subscribe topic= concatenate "publictopic" + "DEVICE_ID"
      mqttclient.subscribe(subtopic);
      Serial.print("Subscibed Topic: ");
      Serial.println(subtopic);


      
    } else {
 
      Serial.print("failed with state ");
      Serial.print(mqttclient.state());
      delay(2000);
 
    }
  }
 
  
  if (netsts>0){ //nextwork status is abnormal
      // upload(String(netsts)); //Upload network status 
        publishjson(String(netsts));
        mySerial.flush();
  }

 mqttclient.loop();
  
  //for test send data without RFID tag reader
  pushbutton.update();
  String test_cno="";
  if(pushbutton.risingEdge()) {
    digitalWrite(blueled, HIGH); //LED OFF
    Serial.print("GPIO0=");
    Serial.println(digitalRead(D3));
    test_cno="0008657047";
    //publishjson("0008657047");
  }


  
  digitalWrite(blueled, LOW); //LED ON


if ((mySerial.available() > 0) || test_cno!=""){
  Serial.println("enter to myserial.ava");

    bool call_extract_tag = false;
  if(test_cno==""){  
   int ssvalue = mySerial.read(); // read 
     
     
    if (ssvalue == -1) { // no data was read
      return;
    }
    if (ssvalue == 2) { // RDM630/RDM6300 found a tag => tag incoming 
      buffer_index = 0;
    } else if (ssvalue == 3) { // tag has been fully transmitted    
       
      
      call_extract_tag = true; // extract tag at the end of the function call
    }
    if (buffer_index >= BUFFER_SIZE) { // checking for a buffer overflow (It's very unlikely that an buffer overflow comes up!)
      Serial.println("Error: Buffer overflow detected!");
      return;
    }
    
    buffer[buffer_index++] = ssvalue; // everything is alright => copy current value to buffer
  }
    if ((call_extract_tag == true || test_cno != "") ) {
      digitalWrite(blueled, HIGH); //LED Off
      if ((buffer_index == BUFFER_SIZE) || (test_cno !="") ) {   
           Serial.println("enter to call tag");
          unsigned tag = extract_tag();
          char buffer[12]; 
          if (test_cno ==""){
            sprintf(buffer, "%010d", tag);   //fixed lenght convert interger to string
          }else{
           tag=99;
           test_cno.toCharArray(buffer, 12);
           Serial.println(buffer);
          }
          String str(buffer);
        if (tag!=oldtag){
        //  if ( tag != tagOld){ //if new card no
             // upload(buffer);
              publishjson(buffer);
         // }
          if (sts==0) {
            oldtag=tag;  
            buzzer1(); //buzzer
           // toneloop();
            }   
          return;
      
        } 
      } else { // something is wrong... start again looking for preamble (value: 2)
        buffer_index = 0;
        return;
      }
  
    }    
     

  }
 else
  {   
    //counter to reset interval
  if (oldtag!=0){  
    stop_read_card_cnt++;
  }else
  {
    stop_read_card_cnt=0;
   }
    if ((oldtag!=0) && (stop_read_card_cnt>10000)){  //interval for upload same card no
      Serial.println(stop_read_card_cnt);
      oldtag=0;
      stop_read_card_cnt=0;
       
    }
  }
  
  
  
}




unsigned extract_tag() {
     char msg_head = buffer[0];
     char *msg_data = buffer + 1; // 10 byte => data contains 2byte version + 8byte tag
   char *msg_data_version = msg_data;
     char *msg_data_tag = msg_data + 2;
    char *msg_checksum = buffer + 11; // 2 byte
    char msg_tail = buffer[13];
    // print message that was sent from RDM630/RDM6300
    Serial.println("--------");
    Serial.print("Message-Head: ");
    Serial.println(msg_head);
    Serial.println("Message-Data (HEX): ");
    for (int i = 0; i < DATA_VERSION_SIZE; ++i) {
      Serial.print(char(msg_data_version[i]));
    }
    Serial.println(" (version)");
    for (int i = 0; i < DATA_TAG_SIZE; ++i) {
      Serial.print(char(msg_data_tag[i]));
    }
    Serial.println(" (tag)");
    Serial.print("Message-Checksum (HEX): ");
    for (int i = 0; i < CHECKSUM_SIZE; ++i) {
      Serial.print(char(msg_checksum[i]));
    }
    Serial.println("");
    Serial.print("Message-Tail: ");
    Serial.println(msg_tail);
    Serial.println("--");
    long tag = hexstr_to_value(msg_data_tag, DATA_TAG_SIZE);
    Serial.print("Extracted Tag: ");
    Serial.println(tag);
    long checksum = 0;
   for (int i = 0; i < DATA_SIZE; i+= CHECKSUM_SIZE) {
      long val = hexstr_to_value(msg_data + i, CHECKSUM_SIZE);
      checksum ^= val;
    }
    Serial.print("Extracted Checksum (HEX): ");
    Serial.print(checksum, HEX);
   if (checksum == hexstr_to_value(msg_checksum, CHECKSUM_SIZE)) { // compare calculated checksum to retrieved checksum
      Serial.print(" (OK)"); // calculated checksum corresponds to transmitted checksum!
    } else {
      Serial.print(" (NOT OK)"); // checksums do not match
    }
    Serial.println("");
    Serial.println("--------");
    return tag;
}
long hexstr_to_value(char *str, unsigned int length) { // converts a hexadecimal value (encoded as ASCII string) to a numeric value
  char *copy =(char*)malloc((sizeof(char) * length) + 1); 
  memcpy(copy, str, sizeof(char) * length);
  copy[length] = '\0'; 
  // the variable "copy" is a copy of the parameter "str". "copy" has an additional '\0' element to make sure that "str" is null-terminated.
  long value = strtol(copy, NULL, 16);  // strtol converts a null-terminated string to a long value
  free(copy); // clean up 
  return value;
}

 
void callback(char* topic, byte* payload, unsigned int length) {
char json[length + 1];

  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
 
  Serial.print("Message:");
 /* for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
     
  }*/

  strncpy (json, (char*)payload, length);
  json[length] = '\0';

  
  
  Serial.println(json);
  
   
  // Decode JSON request  
  StaticJsonBuffer<300> JSONBuffer;   //Memory pool
  JsonObject& parsed = JSONBuffer.parseObject((char*)json); //Parse message 
  
  
  if (!parsed.success())
  {
    Serial.println("parseObject() failed");
    return;
  }


  
 
    sts=parsed["sts"];
    inout=String((const char*)parsed["inout"]);
    cardno=String((const char*)parsed["cardno"]);
    username=String((const char*)parsed["username"]);
    opt=parsed["opt"];
  

 
  Serial.println(sts);
  Serial.println(inout);
  Serial.println(cardno);
  Serial.println(username);
  Serial.println(opt);

  Serial.println();
  Serial.println("-----------------------");


 
}

void publishjson(String cardno){
  StaticJsonBuffer<300> JSONbuffer;
  JsonObject& JSONencoder = JSONbuffer.createObject();
  JSONencoder["SID"]= String(DEVICE_ID);
  JSONencoder["cno"]= String(cardno); 
  JSONencoder["netsts"]= String(netsts) ;
  JSONencoder["ssid"]=String(WiFi.SSID());
if (!JSONencoder.success())
  {
    Serial.println("JSONencoder failed");
    return;
  }


  
  char JSONmessageBuffer[100];
  JSONencoder.printTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
  Serial.print("Sending message to MQTT topic...");
  Serial.println(pubtopic);
  Serial.println(JSONmessageBuffer);
 
  if (mqttclient.publish(pubtopic, JSONmessageBuffer) == true) {
    Serial.println("Success sending message");
  } else {
    Serial.println("Error sending message");
 
  }
 netsts=0; //reset netsts after send to server
  mqttclient.loop();
  Serial.println("-------------");
 netsts = 0 ;
  delay(100);
}
   
