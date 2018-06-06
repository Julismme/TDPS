#include "mbed.h"

Serial pc(USBTX,USBRX);
Serial Hc12(p9,p10);
Serial jy901(p13,p14);

DigitalIn echopinf(p15); //front echo
DigitalOut trigpinf(p16);//front trig
DigitalIn echopin(p17); // right echo
DigitalOut trigpin(p18);// right trig


BusOut move(p19,p20); //0,1,2,3 brake,left,right forward

PwmOut weiyu(p21);
DigitalOut S0(p22); //orange line
DigitalOut S1(p23); //green line
DigitalOut LED(p24); // yellow line
BusOut color(p25,p26);//S2 bule line,S3 purple line
//0-red;1-blue;2-clear;3-green
InterruptIn channal(p27);// used to read pwm ,out brown line



//color sensor
bool once=true;
int r,g,b;
int R,G,B;
float con[3];
int n[4];// white balance start
int ct=1000;// count time
volatile int number = 0;

//timer
Timer t;
Ticker timeup;

struct tm hct;                         // current time will be stored here
char angle[6];
float z;
float zt,zh;


/// Tcs3200 part////////

void setup()  //used to set up the detecting signal
{
    trigpin=0;
    wait_us(2);
    trigpin=1;
    wait_us(11);
    trigpin=0;
}

void setupf()  //used to set up the detecting signal
{
    trigpinf=0;
    wait_us(2);
    trigpinf=1;
    wait_us(11);
    trigpinf=0;
}

float getecho()  // get distance of right side
{
    setup();
    float pulsewidth;
    t.start();
    while(!echopin);
    t.reset();
    while(echopin);
    pulsewidth=t.read_us();
    t.stop();
    float distance=pulsewidth/58;
    return distance;
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
    return distance;
}

////Jy901 part///////////////

float dir(float c)  //get direction
{
    if(jy901.readable()) {
        while(jy901.getc()==0x53) {
            for(int i=0; i<6; i++) {
                angle[i]=jy901.getc();
            }
        }
    }
    c = (angle[5]<<8)|angle[4];
    //pc.printf("%f, ",c);
    c = (c/32768)*180;
    pc.printf("dir=%f, ",c);
    return c;
}

void Lturn(float d)  //turn left
{
    move = 0;//brake
    wait(0.1);
    for(int l=0;l<9;l++){
        zh = dir(zh);
        if(zh<117||zh>120){
            z=zh;
            zt=zh;
        }
    }
    
    move = 1;//left turn
    zh = dir(zh);
    if(zh>zt-10&&zh<zt+10){
        zt=zh;
    }
    if(zt<z-90){
        zt = zt+360;
        }
    while((zt-z)<d) {
        zh = dir(zh);
    if(zh>zt-10&&zh<zt+10){
        zt=zh;
    }
    if(zt<z-90){
        zt = zt+360;
    }
    }
        wait(0.05);
        float ang = zt-z;
        
        pc.printf("L=%f, %f, %f\n\t",z ,zt, ang);
    move = 0;//brake
    wait(0.1);
    for(int l=0;l<9;l++){
        zh = dir(zh);
        if(zh<117||zh>120){
            z=zh;
        }
    }
    pc.printf("Lturn");
}

void Rturn(float d)  //turn right
{
    move = 0;//brake
    wait(0.1);
    for(int l=0;l<9;l++){
        zh = dir(zh);
        if(zh<117||zh>120){
            z=zh;
            zt=zh;
        }
    }
    move = 2;//right turn
    zh = dir(zh);
    if(zh>zt-10&&zh<zt+10){
        zt=zh;
    }
    if(zt>z+90){
            zt=zt-360;
        }
    while((zt-z)>(-d)) {
        zt = dir(zt);
        if(zt>z+90){
            zt=zt-360;
        }
        //pc.printf("%f, %f",zt ,z);
    }
    move = 0;//brake
    wait(0.1);
    z = dir(z);
}

void direction()  // move forward
{
    zt = dir(zt);
    if((zt-z)<=-10) {
        Lturn(z-zt);
    }
    if((zt-z)>=10) {
        Rturn(zt-z);
    }
    move = 3;
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
        if(B>R){
            pc.printf("Blue\n");
        }
        else if(G>R){
            pc.printf("Green\n");
        }
        else if(R>B){
            pc.printf("Red\n");
        }
    LED = 0;
    wait(3);
    }
}
//////main part/////
int main(){// UNCOMMENT BELOW if you want to set time manually 
        hct.tm_year = 2018;     // current year
        hct.tm_mon = 3;         // current month
        hct.tm_mday = 28;        // current day
        hct.tm_hour = 21;       // current hour
        hct.tm_min = 59;        // current minute
        hct.tm_sec = 0;         // current second
        hct.tm_year = hct.tm_year - 1900;   // adjust for tm structure required values
        hct.tm_mon = hct.tm_mon - 1;
        channal.rise(&count);// initial color
        weiyu.period_ms(50);
        weiyu = 0.04;
        
        
        
        
    move = 0;
    wait(3);
    balanced();
    // white balanced end
    
    t.reset();
    t.start();
    while(t.read_ms()<8000){
         z = dir(z);
    }
    t.stop();
    // go to color part

    getcolor();
    // get color

    Lturn(90);
    //Lturn 
    
    t.reset();
    t.start();
    while(t.read_ms()<10000){
        move = 3;
        direction();
    }
    t.stop();
    // go to task 2


    setupf();
    while(getechof()>=15) {
        direction();
    }

    Rturn(90);
    
    setupf();
    while(getechof()>=10) {
        direction();
    }
    // go to chouyulanxia
    
    weiyu = 0.03;//90
    wait(3);
    weiyu = 0.04;//135
    wait(3);
    //wei yu 
    
    
    Lturn(90);
    t.reset();
    t.start();
    while(t.read_ms()<3000){
        move = 3;
        direction();
    }
    t.stop();
    Lturn(90);
    
    t.reset();
    t.start();
    while(t.read_ms()<20000){
        move = 3;
        direction();
    }
    t.stop();
    // go to task 3
    
    
    //Hc12.send()
    Hc12.baud(9600); //Baud rate
        set_time(mktime(&hct));           // set the time
        time_t seconds = time(NULL);
        Hc12.printf("Team Name: TurboT                  ");
        Hc12.printf("Team Member Names:Mingxuan Du,Jiuzhang Jiang,Yang Ni,Boyu Zhou,Han Wang,Zhenhua Cui,Yuxiang Liu,Qiyun Peng,Xinlu Chen,Daye Wei    ");
        Hc12.printf("Time of day: %s ",ctime(&seconds));
    move = 0;
}