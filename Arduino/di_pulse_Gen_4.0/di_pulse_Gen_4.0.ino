/* 
Генератор гальванически развязанных сдвоенных импульсов ( импульсы на разных пинах ).

Программа только для  Arduino Mega2560  v.R3, т.к. только там нашлась
пара свободных 16-ти битных таймеров (можно взять Mega2560 и другой ревизии, 
но тогда придется переопределить в программе выходные пины таймеров).
Импульсы задаются таймерами 3 и 5, 
(пины 2 и 45 соответственно, можно переопределить в небольших пределах).
При необходимости, можно использовать еще один таймер (4).

При необходимости, изменить в программе значения длительность импульсов и задержку между ними.
Изменение частоты реализовано через монитор монитор порта (serial)

При написании программы использовались идеи представленные в:
1.<a href="http://arduino.ru/forum/programmirovanie/etyudy-dlya-nachinayushchikh-blink-i-bez-delay-i-bez-millis" rel="nofollow">http://arduino.ru/forum/programmirovanie/etyudy-dlya-nachinayushchikh-blink-i-bez-delay-i-bez-millis</a>
2.<a href="http://arduino.ru/forum/proekty/generator-s-reguliruemoei-chastotoi-na-arduino" rel="nofollow">http://arduino.ru/forum/proekty/generator-s-reguliruemoei-chastotoi-na-arduino</a>
3.библиотека Timer.h

Загружаете скетч в Мегу 2560 (R3) и на пинах 2 и 45 появляются импульсы. 
*/


#include <limits.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

#define  setDuty() {OCR3B=dutyOnePulse;OCR5B=dutyTwoPulse;}

LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x3f for a 16 chars and 2 line display

unsigned char clockSelectBits;    // переменная для записи в управляющий регистр таймера
unsigned long dutyOnePulse = 56;  // 56 = 7 microSeconds, это длительность первого импульса
unsigned long dutyTwoPulse = 8;   // 8  = 1 microSeconds, это длительность второго импульса
int16_t       PulseDelay = 64465; // -1070 это примерно 70 мкс задержки между импульсами
int8_t        mode = 1;           // режим работы энкодера

int16_t    prescaler[6] = {1,1,8,64,256,1024}; // 5 значений прескалера + оптимальное(нулевое для записи)
uint16_t   counter[6];    // переменная для счетчика, можно обойтись без массива
double     frequency[6];  // переменная для частоты, можно обойтись без массива
double     difference[6]; // переменная для ошибки, можно обойтись без массива
bool       valid;         // двоичная переменная..

double             freq = 200;       // начальное значение частоты следования импульсов в Герцах
volatile int8_t    val = 0;           // переменная для энкодера
volatile boolean   flag = false;      // флаговый автомат

 
void setup() {
  
 pinMode(18,INPUT_PULLUP);      // другой пин энкодера на цифровом пине 18..
 pinMode(3,INPUT_PULLUP);           // кнопка на первом прерывании, пин 3
 attachInterrupt(1, set_mode, LOW); // привязываем 1-е прерывание к функции set_mode()
 pinMode(19,INPUT_PULLUP);          // Энкодер на 4-м прерывании, пин 19 
 attachInterrupt(4, set_val, FALLING);     // привязываем 4-е прерывание к функции set_val()
 
 pinMode(2,OUTPUT);  // выход генератора, импульс 1, таймер 3
 pinMode(45,OUTPUT); // выход генератора, импульс 2, таймер 5
  
 GTCCR=(1<<TSM)|(1<<PSRASY)|(1<<PSRSYNC);
 TCCR3B=0;TCCR5B=0;
 TCCR3A=1<<COM3B1; TCCR5A=1<<COM5B1;
 TCCR3B=(1<<WGM33)|(1<<CS30); TCCR5B=(1<<WGM53)|(1<<CS50);
 setDuty();
 ICR3=3999; ICR5=3999;
 TCNT3=0;  TCNT5=PulseDelay;
 GTCCR=0;
  
 lcd.init();                      // инициализация дисплея
 lcd.backlight();
 set_display();
}

void setPreScaler(double freq)
{ // расчет прескалера, ошибки частоты и значения счетчика
  
  difference[0] = 200;   // это значение "начальной" ошибки между заданной и генерируемой частотой
  
  for ( int8_t i=1; i<6; i++ ){  // в этом цикле производится перебор делителей и выбирается вариант с наименьшей ошибкой
    
    double f0 = floor( F_CPU / ( 2.0 * prescaler[i] * freq ) - 0.5 );
    
    if ( f0 < 0.0 || f0 > UINT_MAX ) { valid = false; }
    else { valid = true; }
    
    counter[i] = (uint16_t) f0;
    frequency[i] = F_CPU / ( 2.0 * prescaler[i] * ( 1.0 + counter[i] ) );
    difference[i] = fabs( frequency[i] - freq );
    
    if ( (difference[i] < difference[0] ) && ( valid == true ) ) 
    { // вариант с наименьшей ошибкой запоминается в нулевом номере массива
      difference[0] = difference[i];
      prescaler[0]  = prescaler[i];
      counter[0]    = counter[i];
      frequency[0]  = frequency[i];         
     }    
  }
  
  TCCR3B = _BV(WGM13); // остановка Таймеров
  TCCR5B = _BV(WGM13);
  
  switch (prescaler[0]) // запись выбранного значения делителя в clockSelectBits
      {   
            case 1:  //выполняется когда  prescaler[0] равно 1
                     clockSelectBits = _BV(CS10);
            break;      
            case 8:  //выполняется когда  prescaler[0] равно 8
                     clockSelectBits = _BV(CS11);
            break;
            case 64: //выполняется когда  prescaler[0] равно 64
                     clockSelectBits = _BV(CS11) | _BV(CS10);
            break;      
            case 256: //выполняется когда  prescaler[0] равно 256                                                     
                     clockSelectBits = _BV(CS12);                                                
            break;
            case 1024: //выполняется когда  prescaler[0] равно 1024                                                    
                     clockSelectBits = _BV(CS12) | _BV(CS10);                                                
            break;    
       }
   ICR3 = counter[0]; //  запись выбранного значения счетчика в ICR3
   ICR5 = counter[0]; //  у нас импульсы с одинаковой частотой..

   TCCR3B &= ~(_BV(CS10) | _BV(CS11) | _BV(CS12));
   TCCR5B &= ~(_BV(CS10) | _BV(CS11) | _BV(CS12)); // обнуление в Таймере5 управляющего регистра TCCR5B  
   
}

void set_mode(void)
{ // по нажатию кнопки, изменяется режим работы энкодера
  detachInterrupt(1);
  flag = true;
}

void set_val(void)
{ // вращение энкодера с учетом направления
  if (digitalRead(18)) 
  { 
    val++; 
  }
  else 
  { 
    val--; 
  }
}

void set_display(void)
{ // вывод информации на дисплей
  lcd.setCursor(0,0);
  lcd.print("                ");
  lcd.setCursor(0,1);
  lcd.print("                ");
  
  lcd.setCursor(0,0);
  lcd.print("M");
  lcd.print(mode);
  lcd.print("  f = ");
  lcd.print(freq);
  
  lcd.setCursor(0,1);
  lcd.print("t=");
  lcd.print(dutyOnePulse);
  lcd.print(",");
  lcd.print(dutyTwoPulse);
  lcd.print(" dt=");
  lcd.print(PulseDelay);
}

void loop() {

if (flag==true) 
{ // обработка нажатия кнопки
  mode++;
  if (mode==5) {mode=1;}
  flag=false;
  attachInterrupt(1, set_mode, LOW);
  set_display();
}


if (val!=0) 
{
  switch (mode) {
    case 1:
      //выполняется, когда mode равно 1
      if (val>0)
      {
        if ((freq + val*100)<14000) {freq = freq + val*100;}        
      }
      else
      {
        if ((freq + val*100)>100) {freq = freq + val*100;}
      }      
      break;
      
    case 2:
      //выполняется когда  mode равно 2
      if (val>0)
      {
        if ((freq + val)<14000) {freq = freq + val;}         
      }
      else
      {
        if ((freq + val)>100) {freq = freq + val;} 
      }      
      break;

    case 3:
      //выполняется когда  mode равно 3
      if (val>0)
      {
        if (dutyOnePulse<70) {dutyOnePulse++;}               
      }
      else
      {
        if (dutyOnePulse>30) {dutyOnePulse--;}
      }      
      break; 

    case 4:
      //выполняется когда  var равно 3
      if (val>0)
      {
        if (PulseDelay<1350) {PulseDelay=PulseDelay+val;}               
      }
      else
      {
        if (PulseDelay>900) {PulseDelay=PulseDelay+val;}
      }      
      break;     
      
  }
  
val=0;

GTCCR=(1<<TSM)|(1<<PSRASY)|(1<<PSRSYNC);
TCCR3B=0;TCCR5B=0;
setPreScaler(freq);
TCCR3B=(1<<WGM33)|(1<<CS30); TCCR5B=(1<<WGM53)|(1<<CS50);
setDuty();
TCNT3=0;  TCNT5=PulseDelay;
GTCCR=0; 

set_display();
  
}

delay(500);  

}
