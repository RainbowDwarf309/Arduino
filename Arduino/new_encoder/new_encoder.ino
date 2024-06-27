#define CLK 42
#define DT 44
#define SW 46
uint16_t shift1 = 0;
uint16_t shift2 = 0;
uint16_t lastStateCLK;
uint16_t StateCLK;
uint16_t StateDT;
byte mode = 0;
//byte n = 1;
void setup() {
  Serial.begin (9600);
  pinMode(CLK, INPUT);
  pinMode(DT, INPUT);
  pinMode(SW, INPUT_PULLUP);
  lastStateCLK = PINL & 128; //old state CLK


  cli();

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

  sei();
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
void loop()
{}
