

//#include "AudioDriver.h"
//mcunode pio to real esp pio
#define D0 16
#define D1 5 // I2C Bus SCL (clock)
#define D2 4 // I2C Bus SDA (data)
#define D3 0
#define D4 2 // Same as "LED_BUILTIN", but inverted logic
#define D5 14 // SPI Bus SCK (clock)
#define D6 12 // SPI Bus MISO 
#define D7 13 // SPI Bus MOSI
#define D8 15 // SPI Bus SS (CS)
#define D9 3 // RX0 (Serial console)
#define D10 1 // TX0 (Serial console)
volatile int toggle;
int onoff;
void inline handler (void){
  
  toggle = (toggle == 1) ? 0 : 1;
  if (onoff==1){
  }else{
   toggle=1;
  }  
 // digitalWrite(BUILTIN_LED, toggle);
    digitalWrite(2, toggle);
  timer0_write(ESP.getCycleCount() + 41660000);
  
}
 
void setup() {
 // pinMode(BUILTIN_LED, OUTPUT);
 pinMode(2, OUTPUT);

  noInterrupts();
  timer0_isr_init();
  timer0_attachInterrupt(handler);
  timer0_write(ESP.getCycleCount() + 41660000);
  interrupts();
}

void loop() {
 onoff=0;
  delay(5000);
   onoff =1;
    delay(5000);
} 
