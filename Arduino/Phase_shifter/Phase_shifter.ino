#define NOP __asm__ ("nop\n\t")

#define CLK 42
#define DT 44
#define SW 46

uint16_t shift1 = 0;
uint16_t shift2 = 10;
uint16_t lastStateCLK;
uint16_t StateCLK;
uint16_t StateDT;
byte mode = 1;

void setup() {

  //настроить пины 2,7,45 на вывод
  pinMode(2, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(45, OUTPUT);
  // настройка выводов энкодера
  pinMode(DT, INPUT_PULLUP);
  pinMode(SW, INPUT);
  pinMode(CLK, INPUT_PULLUP);
  lastStateCLK = PINL & 128; //old state CLK

  cli();


  //настройка таймеров
  GTCCR = (1 << TSM) | (1 << PSRASY) | (1 << PSRSYNC);
  //настройка Timer0
  //set timer0 interrupt at 2kHz
  TCCR0A = 0;// set entire TCCR0A register to 0
  TCCR0B = 0;// same for TCCR0B
  TCNT0  = 0;//initialize counter value to 0
  // set compare match register for 2khz increments
  OCR0A = 155;// = (16*10^6) / (2000*64) - 1 (must be <256)
  // turn on CTC mode
  TCCR0A |= (1 << WGM01);
  // Set CS01 and CS00 bits for 1024 prescaler
  TCCR0B |= (1 << CS02) | (1 << CS00);
  // enable timer compare interrupt
  TIMSK0 |= (1 << OCIE0A);

  //настройка Timer3
  TCCR3A = 0;
  TCCR3B = 0;
  TCCR3A |= (1 << COM3B1);
  TCCR3B = (1 << WGM33) | (1 << CS30);
  ICR3 = 50000;
  OCR3B = 8;
  TCNT3 = 0;

  //настройка Timer4
  TCCR4A = 0;
  TCCR4B = 0;
  TCCR4A |= (1 << COM4B1);
  TCCR4B = (1 << WGM43) | (1 << CS40);
  ICR4 = 53332;
  OCR4B = 8;
  TCNT4 = shift1;

  //настройка Timer5
  TCCR5A = 0;
  TCCR5B = 0;
  TCCR5A |= (1 << COM5B1);
  TCCR5B = (1 << WGM53) | (1 << CS50);
  ICR5 = 53332;
  OCR5B = 8;
  TCNT5 = shift2;
  GTCCR = 0;
  sei(); // включить глобальные прерывания

}

ISR(TIMER0_COMPA_vect) {
  boolean knopka = PINL & (1 << 3); // 0-кнопка нажата, 1-кнопка НЕ нажата.
  if (knopka == 0)
    mode = 0;
  else
    mode = 1;
  StateCLK = PINL & 128; //new state CLK
  //
  if (lastStateCLK != StateCLK) {
    StateDT = PINL & 32;
    if (StateDT != StateCLK) {
      if (mode == 1) {
        shift1 += 1;
      }
      else
        shift1 -= 2;
      if (mode == 0) {
        shift2 += 1;
      }
      else
        shift2 -= 2;
    }
    lastStateCLK = StateCLK;

  }
}

void loop() {




  GTCCR = (1 << TSM) | (1 << PSRASY) | (1 << PSRSYNC);
  //set timer0 interrupt at 2kHz
  TCCR0A = 0;// set entire TCCR0A register to 0
  TCCR0B = 0;// same for TCCR0B
  TCNT0  = 0;//initialize counter value to 0
  // set compare match register for 2khz increments
  OCR0A = 155;// = (16*10^6) / (2000*64) - 1 (must be <256)
  // turn on CTC mode
  TCCR0A |= (1 << WGM01);
  // Set CS01 and CS00 bits for 1024 prescaler
  TCCR0B |= (1 << CS02) | (1 << CS00);
  // enable timer compare interrupt
  TIMSK0 |= (1 << OCIE0A);

  //настройка Timer3
  TCCR3A = 0;
  TCCR3B = 0;
  TCCR3A |= (1 << COM3B1);
  TCCR3B = (1 << WGM33) | (1 << CS30);
  ICR3 = 50000;
  OCR3B = 8;
  TCNT3 = 0;

  //настройка Timer4
  TCCR4A = 0;
  TCCR4B = 0;
  TCCR4A |= (1 << COM4B1);
  TCCR4B = (1 << WGM43) | (1 << CS40);
  ICR4 = 53332;
  OCR4B = 8;
  TCNT4 = shift1;

  //настройка Timer5
  TCCR5A = 0;
  TCCR5B = 0;
  TCCR5A |= (1 << COM5B1);
  TCCR5B = (1 << WGM53) | (1 << CS50);
  ICR5 = 53332;
  OCR5B = 8;
  TCNT5 = shift2;
  GTCCR = 0;


  delay(1000);



}
