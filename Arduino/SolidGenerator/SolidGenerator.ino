#define pack __asm__("nop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\t")
#define Delay __asm__("nop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\t")
uint8_t i = 0;
void setup() {
  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);
  Serial.begin(9600);
  DDRD = B10000000;
  DDRB = B00000111;
  cli();


  TCCR0A = 0;// set entire TCCR0A register to 0
  TCCR0B = 0;// same for TCCR0B
  TCNT0  = 0;//initialize counter value to 0
  //
  OCR0A = 15;
  // turn on CTC mode
  TCCR0A |= (1 << WGM01);
  // No prescaler
  TCCR0B |= (1 << CS00);
  // enable timer compare interrupt
  TIMSK0 |= (1 << OCIE0A);

  sei();


}

ISR(TIMER0_COMPA_vect) {
  static uint8_t counter;
  PORTH |= (1 << 4);
  for (i = 0; i < 25; i++) {
    pack;
  }
  PORTH &= ~(1 << 4);
  for (i = 0; i < 3; i++) {
    Delay;
  }
  PORTB |= (1 << 2);
  for (i = 0; i < 25; i++) {
    pack;
  }
  PORTB &= ~(1 << 2);
  for (i = 0; i < 3; i++) {
    Delay;
  }
  counter++;
  if (counter >= 15)
  {
    Delay;
    PORTD |= 1 << 7;
    for (i = 0; i < 2; i++) {
      pack;
    }
    PORTD &= ~(1 << 7);
    Delay;
    PORTB |= 1 << 0;
    pack;
    PORTB &= ~(1 << 0);
    counter = 0;
    TCCR0A = 0;
    TCCR0B = 0;
    TIMSK0 = 0;
    TCCR1A |= 1 << WGM12;
    TCCR1B |= 1 << CS10;
    TIMSK1 |= (1 << OCIE1A);
    OCR1A = 65500;
    TCNT1  = 0;

  }




}



ISR(TIMER1_COMPA_vect) {

static uint8_t freq;
freq++;
if(freq>=24){
  freq =0;
  TCCR1A = 0;
  TCCR1B = 0;
  TIMSK1 = 0;

  OCR0A = 15;
  // turn on CTC mode
  TCCR0A |= (1 << WGM01);
  // No prescaler
  TCCR0B |= (1 << CS00);
  // enable timer compare interrupt
  TIMSK0 |= (1 << OCIE0A);
  TCNT0  = 0;//initialize counter value to 0
}



}














void loop() {
float sensorValue = digitalRead(7);
Serial.println(sensorValue);

}
