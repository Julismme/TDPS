#include "mbed.h"

Serial pc(USBTX, USBRX);
Timer t;
DigitalIn echopin(p10); // echo
DigitalOut trigpin(p11);// trig

void setup(){       //used to set up the detecting signal
    trigpin=0;
    wait_us(2);
    trigpin=1;
    wait_us(11);
    trigpin=0;
    }

int main() {
   while(1){
   setup();
   float pulsewidth;
   t.start();
   while(!echopin);
   t.reset();
   while(echopin);
   pulsewidth=t.read_us();
   t.stop();
   float distance=pulsewidth/58; 
   wait(3);
   pc.printf("%f\n",distance);
   }
}
