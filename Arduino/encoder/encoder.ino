#define CLK 36
#define DT 38
#define SW 40
boolean butt_flag = false;
boolean butt;
unsigned long last_press;
uint16_t shift1 = 0;
uint16_t aState;
uint16_t aLastState;
byte mode;



void setup() {
  pinMode (CLK, INPUT_PULLUP);
  pinMode (DT, INPUT);
  pinMode (SW, INPUT_PULLUP);
  Serial.begin (9600);
  aLastState = digitalRead(CLK);
}
void loop() {
  //   butt = !digitalRead(SW);
  //   if (butt == 1 && butt_flag ==0 && millis()-last_press >200){
  //    butt_flag=1;
  //    mode++;
  //    Serial.print("mode: ");
  //    Serial.println(mode);
  //    last_press=millis();
  //   }
  //   if (butt==0 && butt_flag==1){
  //    butt_flag =0;
  //   }

  aState = ((PINC & 2) >> 1);
  if (aState != aLastState) {
    DT = PIND & 128;
    if ( digitalRead(DT) != aState  ) {
      shift1 ++;
    } else {
      shift1 --;
    }
    Serial.print("Position: ");
    Serial.println(shift1);
  }
  aLastState = aState;
}
