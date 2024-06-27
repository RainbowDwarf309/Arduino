#include <GyverTimers.h>
#include <GyverEncoder.h>
#define CLK 42
#define DT 44
#define SW 46
#define NOP __asm__ ("nop\n\t")
Encoder enc1(CLK, DT, SW);
volatile uint16_t shift1=0;//сдвиг второго сигнала
volatile uint16_t shift2=0; //сдвиг третьего сигнала
volatile byte mode=1;// режим энкодера



void setup() {
Serial.begin(115200); 
//настроить пины 2,7,45 на вывод
pinMode(2,OUTPUT);
pinMode(7,OUTPUT);
pinMode(45,OUTPUT);
pinMode(DT, INPUT_PULLUP); 
pinMode(SW, INPUT);                                  
pinMode(CLK,INPUT_PULLUP);


GTCCR=(1<<TSM)|(1<<PSRASY)|(1<<PSRSYNC);
//настройка Timer3
TCCR3A=0;
TCCR3B=0;
TCCR3A |=(1<<COM3B1);
TCCR3B = (1<<WGM33)|(1<<CS30);
ICR3 = 53332;
OCR3B=8;
TCNT3=0;
//настройка Timer4
TCCR4A=0;
TCCR4B=0;
TCCR4A |=(1<<COM4B1);
TCCR4B = (1<<WGM43)|(1<<CS40);
ICR4 = 53332;
OCR4B=8;
TCNT4=shift1;
//настройка Timer5
TCCR5A=0;
TCCR5B=0;
TCCR5A |=(1<<COM5B1);
TCCR5B = (1<<WGM53)|(1<<CS50);
ICR5 = 53332;
OCR5B=8;
TCNT5=shift2;
GTCCR=0;
Timer0.enableISR();
sei(); // включить глобальные прерывания
}

ISR(TIMER0_A){ 
  enc1.tick(); 
if(enc1.isClick()){
    mode++;
}
if(mode == 1){
  if(enc1.isRight())
    shift1-=1;
  if(enc1.isLeft())
    shift1+=1;
    }
if(mode == 2){
  if(enc1.isRight())
    shift2-=1;
  if(enc1.isLeft())
    shift2+=1;
}

if(mode==3)
  mode=1;
}



void loop() {
GTCCR=(1<<TSM)|(1<<PSRASY)|(1<<PSRSYNC);
//настройка Timer3
TCCR3A=0;
TCCR3B=0;
TCCR3A |=(1<<COM3B1);
TCCR3B = (1<<WGM33)|(1<<CS30);
ICR3 = 53332;
OCR3B=8;
TCNT3=0;
//настройка Timer4
TCCR4A=0;
TCCR4B=0;
TCCR4A |=(1<<COM4B1);
TCCR4B = (1<<WGM43)|(1<<CS40);
ICR4 = 53332;
OCR4B=8;
TCNT4=shift1;
//настройка Timer5
TCCR5A=0;
TCCR5B=0;
TCCR5A |=(1<<COM5B1);
TCCR5B = (1<<WGM53)|(1<<CS50);
ICR5 = 53332;
OCR5B=8;
TCNT5=shift2;
GTCCR=0;

 delay(1000);
 
}
