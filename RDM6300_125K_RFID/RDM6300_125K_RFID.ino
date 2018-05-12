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
int  sts=9 ,opt;
String inout,usrname,cardno;
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
int uploaded = 0;

const int out_buzzer = D5;
const int blueled = D6;

void setup(){
  pinMode(out_buzzer, OUTPUT);//buzzer
  pinMode(blueled, OUTPUT);//led indicator when singing a note
  

  
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
  




}

 

void upload(String cardno){
    if (client.connect(server, 81)) {  // http server is running on default port 80
      Serial.print("connected Web Server IP: ");
      Serial.println(serverStr);
   
      String method = "GET /mvTimeClock/mvTimeClock.aspx";  // PUT HERE YOUR SERVER URL e.g. from http://192.168.1.11/SensorWriteToFile.php?temp=10.5&hum=58
             method +="?SID=" + String(DEVICE_ID);
             method +="&cno=" + String(cardno); 
             method +="&netsts=" + String(netsts) ;
               method +="&ssid=" + String(WiFi.SSID()) ;
             method += " HTTP/1.1\r\n ";
             method += "Host: " + serverStr +"\r\n";   // http server is running on port 80, so no port is specified
             method += "User-Agent: Mihi IoT 01\r\n";  // PUT HERE YOUR USER-AGENT that can be used in your php program or Apache configuration
             method += "\r\n";
             method += "\r\n";
             Serial.println(method);
         
      client.print(method);  
    
      int i = 0;
      while ((!client.available()) && (i < 10)) {         //Wait up to 10 seconds for server to respond then read response
        delay(10);
        i++;
        }  
        String Line;
      while (client.available()){
         Line = client.readStringUntil('\r');
        //Serial.println(Line);
        }
      readxml(Line); //process readtags from HTML page 
      
      netsts=0; //reset netsts after send to server
      client.stop();
      } 
      else {
        Serial.println("Web Server connection failed");
       
      }
 }
 

void readxml(String content){
  char *starttags[] = { "<sts>", "<inout>", "<cardno>","<username>" ,"<opt>"};
  char *endtags[] = { "</sts>", "</inout>", "</cardno>","</username>", "</opt>"};
  String tagdata[6];
 for (int i=0; i<5; i++ )
  {   
   int startValue = content.indexOf(starttags[i]);
      // Serial.println(startValue);
   int endValue = content.indexOf(endtags[i]);
      //   Serial.println(endValue);
      // Serial.println(starttags[i]);
   int k = strlen(starttags[i]);
   tagdata[i] = (String)content.substring(startValue + k, endValue);    // + 7 to make the index start after the <Value> tag  
    Serial.println(tagdata[i]); 
   }
  
     
     sts=tagdata[0].toInt();
     inout =tagdata[1];
     cardno=tagdata[2];
     usrname=tagdata[3];
     opt =tagdata[4].toInt();
   

}

void loop(){
 // Wait for connection(retry due to connection fail)
  while (wifiMulti.run() != WL_CONNECTED) {
    //delay(200);
    Serial.print(".");
    
    netsts = 2 ;
    uploaded=0;
    digitalWrite(blueled, HIGH); //LED OFF
    delay(100);
    digitalWrite(blueled, LOW); //LED ON
    delay(100);
  }

   


  if (netsts>0){ //nextwork status is abnormal
       upload(String(netsts)); //Upload network status 
       mySerial.flush();
  }
  
  digitalWrite(blueled, LOW); //LED ON


if ((mySerial.available() > 0) ){
    bool call_extract_tag = false;
    
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
    if ((call_extract_tag == true) ) {
      digitalWrite(blueled, HIGH); //LED Off
      if (buffer_index == BUFFER_SIZE) {   
        
          unsigned tag = extract_tag();
          char buffer[12]; 
          sprintf(buffer, "%010d", tag);   //fixed lenght convert interger to string
          String str(buffer);
        if (tag!=oldtag){
        //  if ( tag != tagOld){ //if new card no
              upload(buffer);
              
         // }
          if (sts==0) {
            oldtag=tag;  
            buzzer1(); //buzzer
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

   
