//#include "AudioDriver.h"

volatile int toggle;
int onoff;
void inline handler (void){
  
  toggle = (toggle == 1) ? 0 : 1;
  if (onoff==1){
  }else{
   toggle=1;
  }  
  digitalWrite(BUILTIN_LED, toggle);
  timer0_write(ESP.getCycleCount() + 41660000);
  
}
 
void setup() {
  pinMode(BUILTIN_LED, OUTPUT);

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
