int pinButton = 12; // Пин кнопки
int pinLED = 13;    // Пин светодиода

void setup() {
  pinMode(pinButton, INPUT_PULLUP);
  pinMode(pinLED, OUTPUT);
}

bool flag       = LOW;
bool lastButton = LOW;

void loop() {
  int currentButton = digitalRead(pinButton);   // Считываем значение пина кнопки
  if (lastButton == LOW && currentButton == HIGH) { // Если состояние кнопки "нажата" - HIGH, а предыдущее LOW - фиксируем событие нажатия кнопки
    flag = !flag;                               // Меняем флаг статуса светодиода
    digitalWrite(pinLED, flag);                 // В зависимости от флага, зажигаем или гасим светодиод
  }
  lastButton = currentButton;                   // Запоминаем последнее состояние кнопки
}