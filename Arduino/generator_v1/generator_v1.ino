// работает на ARDUINO MEGA
// может быть добавить EEPROM

#include <LiquidCrystal_I2C.h>


// сигналы(импульсы основной/поджиг)
#define main_signal 7      // 1мкс
#define ignition_signal 6  //5мкс
// кнопки уменьшения/увеличения частоты в цуге/частоты в пачке/импульсов в пачке
#define decrease_button 12
#define increase_button 13
// кнопка уменьшения/увеличения количества импульсов в пачке
#define mode_button 11




uint32_t f_cpu = 16000000;  // тактовая частота
uint8_t prescaler = 1;
uint16_t frequency_1_100[] = { 7999, 3999, 2665, 1999, 1599, 1332, 1141, 999, 887, 799, 532, 399, 319, 265, 227, 199, 176, 159, 144, 132, 122, 113, 105, 99, 93, 87, 83, 79 };  // набор значений для регулировки частоты в пределах 1-100кГц
uint16_t frequensy_5_50[] = { 49999, 24999, 16665, 12499, 9999, 8332, 7141, 6249, 5554, 4999 };                                                                                 // набор значений для регулировки частоты в пределах 5-50Гц
uint8_t f_case_1_100 = 11;                                                                                                                                                      // если меняем стартовую частоту freq, то эту переменную тоже надо поменять в соответсвии с индексом значения freq в массиве frequency_1_100
uint8_t f_case_5_50 = 0;
uint16_t freq_pack = frequensy_5_50[f_case_5_50];  //старт с 5Гц
uint16_t freq = frequency_1_100[f_case_1_100];     // старт с 20кГц                                                                                                                                                     // частота цуга 30Гц при старте
uint8_t fp = 5;

bool flag_inc = LOW;  // инкремент частоты
bool lastButton_inc = LOW;
bool flag_dec = LOW;  // декремент частоты
bool lastButton_dec = LOW;
bool flag_mode = LOW;  // смена режима
bool lastButton_mode = LOW;
uint8_t MODE = 2;  // MODE = 0 - меняет частоту импульсов в цуге; MODE = 1 - меняет количество импульсов в цуге; MODE = 2 - меняет частоту цуга
uint8_t PULSES = 10;


LiquidCrystal_I2C lcd(0x27, 16, 2);  // Устанавливаем дисплей

void setup() {

  // настройка lcd
  lcd.init();       // инициализация LCD дисплея
  lcd.backlight();  // включение подсветки дисплея

  pinMode(ignition_signal, OUTPUT);        // поджиг
  pinMode(main_signal, OUTPUT);            // основной
  pinMode(decrease_button, INPUT_PULLUP);  //кнопка уменьшения частоты
  pinMode(increase_button, INPUT_PULLUP);  // кнопка увеличения частоты
  pinMode(mode_button, INPUT_PULLUP);      //кнопка уменьшения количества импульсов

  GTCCR = (1 << TSM) | (1 << PSRASY) | (1 << PSRSYNC);
  //настройка Timer4
  TCCR4A = 0;  // установить регистры в 0
  TCCR4B = 0;
  TCCR4A = (1 << COM4A1) | (1 << COM4B1);  // запуск импульсов на 6 и 7 пинах
  TCCR4B = (1 << WGM43) | (1 << CS40);     // режим PWM Phase and Freauency control и установка prescaler 1
  ICR4 = freq;
  OCR4A = 48;              // длительность
  OCR4B = 16;              // длительность
  TCNT4 = 0;               // фаза
  TIMSK4 |= (1 << TOIE4);  //разрешить прерывание при переполнении(OVF) почитать документацию по режиу  PWM Phase and Freauency control если не понятно зачем
  GTCCR = 0;
}


void loop() {

  int freq_work = (f_cpu / 2) / (prescaler * (1 + freq)) / 1000;

  //инкремент
  int currentButton_inc = digitalRead(increase_button);      // Считываем значение пина кнопки
  if (lastButton_inc == LOW && currentButton_inc == HIGH) {  // Если состояние кнопки "нажата" - HIGH, а предыдущее LOW - фиксируем событие нажатия кнопки
    flag_inc = !flag_inc;
    lcd.clear();
    if (MODE == 0) {
      if (f_case_1_100 < 27) {
        f_case_1_100++;  // увеличиваем частоту частоту
      }
      cli();
      freq = frequency_1_100[f_case_1_100];
      TCCR4B = (1 << WGM43) | (1 << CS40);  // Set 1 prescaler
      ICR4 = freq;
      sei();
    }
    if (MODE == 1) {
      if (PULSES < 200) {
        cli();
        PULSES += 5;  // увеличиваем пачку
        sei();
      }
    }
    if (MODE == 2) {
      if (f_case_5_50 < 9) {
        f_case_5_50++;
        fp += 5;
      }
      cli();
      freq_pack = frequensy_5_50[f_case_5_50];
      OCR1A = freq_pack;
      sei();
    }
  }
  lastButton_inc = currentButton_inc;  // Запоминаем последнее состояние кнопки


  // декремент
  int currentButton_dec = digitalRead(decrease_button);      // Считываем значение пина кнопки
  if (lastButton_dec == LOW && currentButton_dec == HIGH) {  // Если состояние кнопки "нажата" - HIGH, а предыдущее LOW - фиксируем событие нажатия кнопки
    flag_dec = !flag_dec;
    lcd.clear();

    if (MODE == 0) {
      if (f_case_1_100 > 0) {
        f_case_1_100--;  // уменьшаем частоту в пачке
      }
      cli();
      freq = frequency_1_100[f_case_1_100];
      TCCR4B = (1 << WGM43) | (1 << CS40);  // Set 1 prescaler
      ICR4 = freq;
      sei();
    }

    if (MODE == 1) {
      if (PULSES > 5) {
        cli();
        PULSES -= 5;  // уменьшаем пачку
        sei();
      }
    }

    if (MODE == 2) {
      if (f_case_5_50 > 0) {
        f_case_5_50--;
        fp -= 5;
      }
      cli();
      freq_pack = frequensy_5_50[f_case_5_50];
      OCR1A = freq_pack;
      sei();
    }
  }
  lastButton_dec = currentButton_dec;  // Запоминаем последнее состояние кнопки



  // изменение режима
  int currentButton_mode = digitalRead(mode_button);           // Считываем значение пина кнопки
  if (lastButton_mode == LOW && currentButton_mode == HIGH) {  // Если состояние кнопки "нажата" - HIGH, а предыдущее LOW - фиксируем событие нажатия кнопки
    flag_mode = !flag_mode;
    lcd.clear();
    MODE++;
    if (MODE > 2) {
      MODE = 0;
    }
  }
  lastButton_mode = currentButton_mode;  // Запоминаем последнее состояние кнопки



  if (MODE == 0) {
    lcd.setCursor(0, 0);
    lcd.print("Frequency:");  
    lcd.setCursor(10, 0);
    String freqstr = String(String((freq_work)) + "kHz");
    lcd.print(freqstr);  // Выводим частоту импульсов в цуге
  }
  if (MODE == 1) {
    lcd.setCursor(0, 0);
    lcd.print("Pulses:");  
    lcd.print(PULSES);     // Выводим количество импульсов
  }
  if (MODE == 2) {
    lcd.setCursor(0, 0);
    lcd.print("Pack_freq:");  
    // lcd.setCursor(10, 0);
    String pack_str = String(String((fp)) + "Hz");
    lcd.print(pack_str);  // Выводим частоту
  }
  lcd.setCursor(0, 1);
  lcd.print("MODE:");  // Выводим номер режима
  lcd.print(MODE);     
}


ISR(TIMER1_COMPA_vect) {
  cli();
  GTCCR = (1 << TSM) | (1 << PSRASY) | (1 << PSRSYNC);
  TCCR1A = 0;  // установить регистры в 0
  TCCR1B = 0;
  TCCR4A = 0;  // установить регистры в 0
  TCCR4B = 0;
  TCCR4A = (1 << COM4A1) | (1 << COM4B1);  // запуск импульсов на 6 и 7 пинах
  TCCR4B = (1 << WGM43) | (1 << CS40);     // режим PWM Phase and Freauency control и установка prescaler 1
  ICR4 = freq;
  OCR4A = 48;              // длительность
  OCR4B = 16;              // длительность
  TCNT4 = 0;               // фаза
  TIMSK4 |= (1 << TOIE4);  //разрешить прерывание при переполнении(OVF) почитать документацию по режиу  PWM Phase and Freauency control если не понятно зачем
  GTCCR = 0;
  sei();
}


ISR(TIMER4_OVF_vect) {
  static uint8_t counter;
  counter++;
  if (counter > PULSES - 1) {
    cli();
    counter = 0;
    GTCCR = (1 << TSM) | (1 << PSRASY) | (1 << PSRSYNC);
    TCCR4A = 0;  // установить регистры в 0
    TCCR4B = 0;

    //Таймер1
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1 = 0;
    OCR1A = freq_pack;
    TCCR1A |= (1 << WGM12);  // режим СТС
    TCCR1B |= (1 << CS11);   // Set 64 prescaler
    TCCR1B |= (1 << CS10);
    TIMSK1 |= (1 << OCIE1A);
    GTCCR = 0;
    sei();
  }
}
