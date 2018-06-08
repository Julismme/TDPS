#include "mbed.h"

Serial pc(USBTX,USBRX);

DigitalOut weiyu(p5);
Serial jy901(p9,p10);
Serial Hc12(p13,p14);
DigitalOut l1(LED1);// right trig
DigitalOut l2(LED2);// right trig
DigitalOut l3(LED3);// right trig
DigitalOut l4(LED4);// right trig

DigitalIn echopinf(p11); //front echo
DigitalOut trigpinf(p12);//front trig

DigitalOut S0(p15); //
DigitalOut S1(p16); //
DigitalOut LED(p17); // 
BusOut color(p19,p20);//S2 bule line,S3 purple line
//0-red;1-blue;2-clear;3-green
InterruptIn channal(p18);// used to read pwm ,out brown line

//Pins controling the right motors:
//p21 p22 p23 
DigitalOut motor_Right_in1(p21);
DigitalOut motor_Right_in2(p22);
PwmOut motor_Right_pwm(p23);

//Pins controling the left motors
//p26 p25 p24
DigitalOut motor_Left_in1(p25);
DigitalOut motor_Left_in2(p24);
PwmOut motor_Left_pwm(p26);

/////////////////////////Drinving part///////////////////

void move(float t,float dt,float dif)
{
    motor_Right_in1 = 1;
    motor_Right_in2 = 0;
    motor_Right_pwm.period_us(100);
    motor_Right_pwm = dt-dif;

    motor_Left_in1 = 1;
    motor_Left_in2 = 0;
    motor_Left_pwm.period_us(100);
    motor_Left_pwm = dt+dif;
    wait(t);
}

void Lturn(float dif)
{
    motor_Right_in1 = 1;
    motor_Right_in2 = 0;
    motor_Right_pwm.period_us(100);
    motor_Right_pwm = dif;

    motor_Left_in1 = 0;
    motor_Left_in2 = 1;
    motor_Left_pwm.period_us(100);
    motor_Left_pwm = dif;
}

void Rturn(float dif)
{
    motor_Right_in1 = 0;
    motor_Right_in2 = 1;
    motor_Right_pwm.period_us(100);
    motor_Right_pwm = dif;

    motor_Left_in1 = 1;
    motor_Left_in2 = 0;
    motor_Left_pwm.period_us(100);
    motor_Left_pwm = dif;
}

void brake(float t)
{  
    motor_Right_in1 = 1;
    motor_Right_in2 = 1;
    motor_Right_pwm.period_us(100);
    motor_Right_pwm = 0;

    motor_Left_in1 = 1;
    motor_Left_in2 = 1;
    motor_Left_pwm.period_us(100);
    motor_Left_pwm = 0;
    wait(t);
}

//color sensor
int r,g,b;
int R,G,B;
float con[3];
int n[4];// white balance start
int ct=1000;// count time
volatile int number = 0;

//timer
Timer t;
Timer timeup;

struct tm hct;                         // current time will be stored here
unsigned char angle[11];
int sign = 0;
int counter = 0;
float z;
float zt;


/// Tcs3200 part////////

void setupf()  //used to set up the detecting signal
{
    trigpinf=0;
    wait_us(2);
    trigpinf=1;
    wait_us(11);
    trigpinf=0;
}

float getechof()  // get distance of front
{
    setupf();
    float pulsewidth;
    t.start();
    while(!echopinf);
    t.reset();
    while(echopinf);
    pulsewidth=t.read_us();
    t.stop();
    float distance=pulsewidth/58;
    pc.printf("%f\n",distance);
    return distance;
}

////Jy901 part///////////////

float dir(float c){
    if(sign){  
    pc.printf("sign:%d\n",sign);
    sign=0;
    c = short ((angle[7]<<8)|angle[6]);
    c = (c/32768.0)*180;
    pc.printf("%f\n\t",c);
    return c;
    }
}

void recieve(){
    angle[counter]=(unsigned char)jy901.getc();
    if(counter==0&&angle[0]!=0x55) return;                  
    counter++;       
    if(counter==11){    
       counter=0;               
       sign=1;
    }
}

void direction(float z,int c){
    float angm;
    zt=dir(zt);
    if(z<-90&&zt>90){
        zt=zt-360;
        }
    if(z>90&&zt<-90){
        zt=zt+360;
        }
    timeup.start();
    timeup.reset();
    while(timeup.read_ms()<c){
    angm = 0.5/180*(zt-z);
    move(0,0.5,angm);
    wait(0.1);
    zt=dir(zt);
        if(z<-90&&zt>90){
            zt=zt-360;
        }
        if(z>90&&zt<-90){
            zt=zt+360;
        }
    }
}

//color sensor
void count(){
    ++number;
    }
    
void getchannal(int time){ // count the number of pulse in one second
    number = 0;
    wait_ms(time);
    pc.printf("number=%d\n",number);
}

void balanced(){
    LED=1;
    S0=0;
    S1=1;
    
    for (int i=0;i<4;i++){
        if(i!=2){
            color=i;
            getchannal(ct);
            n[i]=number;
        }
    }
    r=n[0];
    b=n[1];
    g=n[3];
    pc.printf("r=%d,g=%d,b=%d\n",r,g,b);
    con[0]=255.0/r; // get the ratio factor for red
    con[1]=255.0/b; // get the ratio factor for green
    con[2]=255.0/g; // get the ratio factor for blue
    pc.printf(" white balance sucessful");
}

void getcolor(){
    LED=1;
    S0=0;
    S1=1;
    
    for (int i=0;i<4;i++){
        if(i!=2){
            color=i;
            getchannal(ct);
            n[i]=number;
        }
    }
    r=n[0];
    b=n[1];
    g=n[3];
    R=con[0]*n[0];
    B=con[1]*n[1];
    G=con[2]*n[3];
    pc.printf("R=%d,G=%d,B=%d\n",R,G,B);
        if(B>R&&B>G){
            l3 = 1;
            pc.printf("Blue\n");
        }
        else if(G>R&&G>B){
            l2 = 1;
            pc.printf("Green\n");
        }
        else if(R>G&&R>B){
            l1 = 1;
            pc.printf("Red\n");
        }
    LED = 0;
    wait(3);
}    


//////main part/////
int main(){// UNCOMMENT BELOW if you want to set time manually
        brake(0);
        jy901.attach(&recieve, jy901.RxIrq);
        hct.tm_year = 2018;     // current year
        hct.tm_mon = 6;         // current month
        hct.tm_mday = 9;        // current day
        hct.tm_hour = 8;       // current hour
        hct.tm_min = 59;        // current minute
        hct.tm_sec = 0;         // current second
        hct.tm_year = hct.tm_year - 1900;   // adjust for tm structure required values
        hct.tm_mon = hct.tm_mon - 1;
        channal.rise(&count);// initial color
        float zb;
    wait(3);  
    
    balanced();
    // white balanced
    
    z=dir(z);
    for(int i=0;i<5;i++){
        z=(dir(z)+z)/2;
    }
    zt=z;
    zb=z;
    
    direction(z,7700);// 243 cm
    brake(0.1);
    // go to color part
    
    z=dir(z);
    for(int i=0;i<5;i++){
        z=(dir(z)+z)/2;
    }
    
    getcolor();
    // get color
    
    z=dir(z);
    for(int i=0;i<5;i++){
        z=(dir(z)+z)/2;
    }
    if(l1==1){
        Lturn(0.5);
        wait(0.65);
        brake(2);
        z=dir(z);
        for(int i=0;i<5;i++){
            z=(dir(z)+z)/2;
        }//L45
        
        direction(z,10000);// 322 cm
        brake(0.1);
        
        Lturn(0.5);
        wait(1.3);
        brake(2);
        z=dir(z);
        for(int i=0;i<5;i++){
            z=(dir(z)+z)/2;
        }//L90
        
        direction(z,10000);// 322 cm
        brake(0.1);
        
        Rturn(0.5);
        wait(0.65);
        brake(2);
        z=dir(z);
        for(int i=0;i<5;i++){
            z=(dir(z)+z)/2;
        }//R45
    }
    if(l2==1){
        Lturn(0.5);
        wait(1.3);
        brake(2);//L90
        
        z=dir(z);
        for(int i=0;i<5;i++){
            z=(dir(z)+z)/2;
        }
        
        direction(z,14000);// 322 cm
        brake(3);
    }
    if(l3==1){
        Lturn(0.5);
        wait(1.95);
        brake(2);
        z=dir(z);
        for(int i=0;i<5;i++){
            z=(dir(z)+z)/2;
        }//L135
        
        direction(z,10000);// 322 cm
        brake(0.1);
        
        Rturn(0.5);
        wait(1.3);
        brake(2);
        z=dir(z);
        for(int i=0;i<5;i++){
            z=(dir(z)+z)/2;
        }//R90
        
        direction(z,7500);// 322 cm
        brake(0.1);
        
        Lturn(0.5);
        wait(0.65);
        brake(2);
        z=dir(z);
        for(int i=0;i<5;i++){
            z=(dir(z)+z)/2;
        }//L45
    }
    brake(1.5);
    //Lturn 
    
    z=dir(z);
    for(int i=0;i<9;i++){
        z=(dir(z)+z)/2;
    }
    direction(z,10000);
    // go to task 2
    
    setupf();
    while(getechof()>=40) {
        direction(z,500);
    }
    brake(1);
    // dao hua tan
    
    Rturn(0.7);
    wait(1);
    brake(2);
    
    //R90
    //hua tan you zhuan
    
    setupf();
    while(getechof()>=40) {
        direction(zb,500);
    }
    brake(2);
    //dao lan gan
    
    Lturn(0.7);
    wait(1);
    brake(2);
    //lan gan zuo zhuan
    
    z=dir(z);
    for(int i=0;i<9;i++){
        z=(dir(z)+z)/2;
    }
    
    setupf();
    while(getechof()>=40) {
        direction(z,500);
    }
    brake(2);
    //becon
    
    Lturn(0.7);
    wait(1);
    brake(2);
    // becon you zhuan
    
    setupf();
    while(getechof()>=15) {
        direction(zb,500);
    }
    brake(2);
    //kai shi wei yu 
    
    for(int i=0;i<99;i++){
        weiyu=1;
        wait_us(700);
        weuiyu=0;
        wait_ms(20);
        }
    //weiyu
    
    Lturn(0.7);
    wait(1);
    brake(2);
    //zuozhuan
    
    Lturn(0.7);
    wait(1);
    brake(2);
    //diao tou
    
    z=dir(z);
    direction(z,15000);
    brake(3);
    // go to task 3
    
    //Hc12.send()
    Hc12.baud(9600); //Baud rate
        set_time(mktime(&hct));           // set the time
        time_t seconds = time(NULL);
        Hc12.printf("Team Name: TurboT                  ");
        Hc12.printf("Team Member Names:Mingxuan Du,Jiuzhang Jiang,Yang Ni,Boyu Zhou,Han Wang,Zhenhua Cui,Yuxiang Liu,Qiyun Peng,Xinlu Chen,Daye Wei    ");
        Hc12.printf("Time of day: %s ",ctime(&seconds));
    
    brake(1);
    // send message
        
    z=dir(z);
    direction(z,5000);
    //end
}
