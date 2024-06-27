void setup(){ 
 // Serial.begin(115200);
  pinMode(13,OUTPUT);
  // Clear OC1A on Compare Match / Set OC1A at Bottom; Wave Form Generator: Fast PWM 14, Top = ICR1
TCCR1A = 0;   // установить регистры в 0
TCCR1B = 0;
OCR1A=16000;
TCCR1A |= (1<<COM1C0);
TCCR1B |= (1 << WGM12);//CTC mode
TCCR1B |=(1<<CS10);// prescaler 1024
TCCR1B |=(1<<CS12);
TIMSK1 |= (1 << OCIE1C); 
sei();
} 
void loop() { 

} 
