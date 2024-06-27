// ARDUINO MEGA
// может быть добавить EEPROM

#include <LiquidCrystal_I2C.h>

#define pack __asm__("nop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\t")
#define Delay __asm__("nop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\t")

// сигналы(импульсы)
#define main_signal 9
#define ignition_signal 8
// кнопки уменьшения/увеличения частоты
#define decrease_button 12
#define increase_button 13
// кнопки уменьшения/увеличения количества импульсов в пачке
#define plus_button 22
#define minus_button 24


uint16_t freq = 1999;                                                                                                                                                            // старт с 1кГц
uint32_t f_cpu = 16000000;                                                                                                                                                       // тактовая частота
uint16_t prescaler = 8;                                                                                                                                                          // по дефолту 8, будет меняться в ходе программы
uint16_t frequency_1_100[] = { 1999, 999, 665, 499, 399, 332, 284, 249, 221, 1599, 1065, 799, 639, 532, 456, 399, 354, 319, 289, 265, 245, 227, 212, 199, 187, 176, 167, 159 };  // набор значений для регулировки частоты в пределах 1-100кГц

uint8_t f_case = 0;
float freq_work = f_cpu / (prescaler * (1 + freq));

bool flag_inc = LOW;  // инкремент частоты
bool lastButton_inc = LOW;
bool flag_dec = LOW;  // декремент частоты
bool lastButton_dec = LOW;


bool flag_plus = LOW;  // увеличение количества импульсов в пачке
bool lastButton_plus = LOW;
bool flag_minus = LOW;  // уменьшение количества импульсов в пачке
bool lastButton_minus = LOW;
uint8_t PULSES = 15;

String message;

// LiquidCrystal lcd(7, 6, 5, 4, 3, 2);  // (RS, E, DB4, DB5, DB6, DB7)
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Устанавливаем дисплей

void setup() {
  // настройка lcd
  lcd.init();       // инициализация LCD дисплея
  lcd.backlight();  // включение подсветки дисплея


  pinMode(ignition_signal, OUTPUT);        // поджиг
  pinMode(main_signal, OUTPUT);            // основной
  pinMode(decrease_button, INPUT_PULLUP);  //кнопка уменьшения частоты
  pinMode(increase_button, INPUT_PULLUP);  // кнопка увеличения частоты
  pinMode(plus_button, INPUT_PULLUP);      //кнопка уменьшения частоты
  pinMode(minus_button, INPUT_PULLUP);     // кнопка увеличения частоты

  cli();
  // Таймер1 используется для обработки кнопок
  TCCR1A = 0;               // set entire TCCR3A register to 0
  TCCR1B = 0;               // same for TCCR3B
  TCNT1 = 0;                //initialize counter value to 0
  OCR1A = 62499;            // 10 Гц
  TCCR1A |= (1 << WGM11);   //режим СТС
  TCCR1B |= (1 << CS12);    // Set 256 prescaler
  TIMSK1 |= (1 << OCIE1A);  //пока оставим, мб убрать надо
  // Таймер3 используется для импульсов
  TCCR3A = 0;  // set entire TCCR3A register to 0
  TCCR3B = 0;  // same for TCCR3B
  TCNT3 = 0;   //initialize counter value to 0
  OCR3A = freq;
  TCCR3A |= (1 << WGM31);   //режим СТС
  TCCR3B |= (1 << CS31);    // Set 8 prescaler
  TIMSK3 |= (1 << OCIE3A);  //пока оставим, мб убрать надо
  sei();
}


void loop() {
  freq_work = int(f_cpu / (prescaler * (1 + freq)) / 1000);

  // инкремент
  int currentButton_inc = digitalRead(increase_button);      // Считываем значение пина кнопки
  if (lastButton_inc == LOW && currentButton_inc == HIGH) {  // Если состояние кнопки "нажата" - HIGH, а предыдущее LOW - фиксируем событие нажатия кнопки
    flag_inc = !flag_inc;
    lcd.clear();
    cli();
    if (f_case < 27) {
      f_case++;  // увеличиваем частоту частоту
    }


    if (f_case < 9) {
      // lcd.clear();
      prescaler = 8;
      freq = frequency_1_100[f_case];
      TCCR3B |= (1 << CS31);  // Set 8 prescaler
      OCR3A = freq;
    } else {
      // lcd.clear();
      prescaler = 1;
      freq = frequency_1_100[f_case];
      TCCR3B |= (1 << CS30);  // Set 1 prescaler
      OCR3A = freq;
    }
    sei();
  }
  lastButton_inc = currentButton_inc;  // Запоминаем последнее состояние кнопки


  // декремент
  int currentButton_dec = digitalRead(decrease_button);      // Считываем значение пина кнопки
  if (lastButton_dec == LOW && currentButton_dec == HIGH) {  // Если состояние кнопки "нажата" - HIGH, а предыдущее LOW - фиксируем событие нажатия кнопки
    flag_dec = !flag_dec;
    lcd.clear();
    cli();
    if (f_case > 0) {
      f_case--;  // уменьшаем частоту
    }


    if (f_case < 9) {
      prescaler = 8;
      freq = frequency_1_100[f_case];
      TCCR3B |= (1 << CS31);  // Set 8 prescaler
      OCR3A = freq;
    } else {
      prescaler = 1;
      freq = frequency_1_100[f_case];
      TCCR3B |= (1 << CS30);  // Set 1 prescaler
      OCR3A = freq;
    }
    sei();
  }
  lastButton_dec = currentButton_dec;  // Запоминаем последнее состояние кнопки

  // увеличение пачки импульсов
  int currentButton_plus = digitalRead(plus_button);           // Считываем значение пина кнопки
  if (lastButton_plus == LOW && currentButton_plus == HIGH) {  // Если состояние кнопки "нажата" - HIGH, а предыдущее LOW - фиксируем событие нажатия кнопки
    flag_plus = !flag_plus;
    cli();
    if (PULSES < 26) {
      PULSES++;  // увеличиваем пачку
    }
    sei();
  }
  lastButton_plus = currentButton_plus;  // Запоминаем последнее состояние кнопки

  // уменьшение пачки импульсов
  int currentButton_minus = digitalRead(minus_button);           // Считываем значение пина кнопки
  if (lastButton_minus == LOW && currentButton_minus == HIGH) {  // Если состояние кнопки "нажата" - HIGH, а предыдущее LOW - фиксируем событие нажатия кнопки
    flag_minus = !flag_minus;
    lcd.clear();
    cli();
    if (PULSES > 5) {
      PULSES--;  // уменьшаем пачку
    }
    sei();
  }
  lastButton_minus = currentButton_minus;  // Запоминаем последнее состояние кнопки


  lcd.setCursor(0, 0);
  lcd.print("Frequency:");  // Выводим текст
  lcd.setCursor(10, 0);
  String freqstr = String(String(int(freq_work)) + "kHz");
  lcd.print(freqstr);  // Выводим текст
  lcd.setCursor(0, 1);
  lcd.print("Pulses:");  // Выводим текст
  lcd.print(PULSES);     // Выводим текст
}



ISR(TIMER1_COMPA_vect) {
  TCCR3A = 0;  // set entire TCCR3A register to 0
  TCCR3B = 0;  // same for TCCR3B
  TCNT3 = 0;   //initialize counter value to 0
  OCR3A = freq;
  TCCR3A |= (1 << WGM31);                    //режим СТС
  if (prescaler = 8) TCCR3B |= (1 << CS31);  // Set 8 prescaler
  else TCCR3B |= (1 << CS30);                // Set 1 prescaler
  TIMSK3 |= (1 << OCIE3A);                   //пока оставим, мб убрать надо
}


//  импульсы
ISR(TIMER3_COMPA_vect) {
  cli();
  TCCR1B &= ~(1 << CS12);  // вырубаем таймер 1

  static uint8_t counter;
  PORTH |= (1 << 5);  //основной сигнал
  for (uint8_t i = 0; i < 25; i++) {
    pack;
  }
  PORTH &= ~(1 << 5);
  for (uint8_t i = 0; i < 3; i++) {
    Delay;
  }
  PORTH |= (1 << 6);  // поджиг
  for (uint8_t i = 0; i < 25; i++) {
    pack;
  }
  PORTH &= ~(1 << 6);
  for (uint8_t i = 0; i < 3; i++) {
    Delay;
  }
  counter++;
  if (counter >= PULSES) {

    TCCR3A = 0;  // set entire TCCR3A register to 0
    TCCR3B = 0;  // same for TCCR3B
    TCNT3 = 0;   //initialize counter value to 0
    TCCR3A |= (1 << WGM31);   //режим СТС
    TCCR3B &= ~(1 << CS31);    // вырубаем таймер 3
    TIMSK3 |= (1 << OCIE3A);  //пока оставим, мб убрать надо


    counter = 0;
    TCCR1A = 0;               // set entire TCCR3A register to 0
    TCCR1B = 0;               // same for TCCR3B
    TCNT1 = 0;                //initialize counter value to 0
    OCR1A = 62499;            // 10 Гц
    TCCR1A |= (1 << WGM11);   //режим СТС
    TCCR1B |= (1 << CS12);    // Set 256 prescaler
    TIMSK1 |= (1 << OCIE1A);  //пока оставим, мб убрать надо
  }
  sei();
}