#include <Bounce2.h>

#define BUTTON_PIN_1 12
#define BUTTON_PIN_2 11
#define LED_PIN 9
#define VOLTAGE_PIN 14

int powers[] = {1, 2, 4, 8, 16, 32, 64, 128, 255};
int selectedPower = 5;
int actualPower = powers[selectedPower];
bool powerCanBeChanged = true;

unsigned long lastCheckedVoltage;
unsigned long voltageCheckInterval = 2L * 1000;
double voltage;
double infoVoltage = 3.6*6;
double warningVoltage = 3.4*6;

Bounce debouncer1 = Bounce(); 
Bounce debouncer2 = Bounce(); 

void setup() {
  Serial.begin(9600);
  
  pinMode(BUTTON_PIN_1, INPUT_PULLUP);
  debouncer1.attach(BUTTON_PIN_1);
  debouncer1.interval(5);
  
  pinMode(BUTTON_PIN_2, INPUT_PULLUP);
  debouncer2.attach(BUTTON_PIN_2);
  debouncer2.interval(5);

  pinMode(LED_PIN, OUTPUT);
  analogWrite(LED_PIN, actualPower);

  pinMode(VOLTAGE_PIN, INPUT);
 
  TCCR1B = TCCR1B & B11111000 | B00000001;    // set timer 1 divisor to 1 = 31372.55 Hz

}

void loop() {
  debouncer1.update();
  debouncer2.update();
  int value1 = debouncer1.read();
  int value2 = debouncer2.read();

  if (value1 == LOW && powerCanBeChanged ) {
    selectedPower++;
    powerCanBeChanged = false;
  } 
  if (value2 == LOW && powerCanBeChanged ) {
    selectedPower--;
    powerCanBeChanged = false;
  }
  if (value1 == HIGH && value2 == HIGH) {
    powerCanBeChanged = true;
  }

  if (selectedPower > 8) selectedPower = 8;
  if (selectedPower < 0) selectedPower = 0;

  actualPower = powers[selectedPower];
  analogWrite(LED_PIN, actualPower);

  if (lastCheckedVoltage + voltageCheckInterval < millis()) {
    voltageCheckInterval = 3*60L*1000;
    Serial.print("Power set to: ");
    Serial.println(actualPower);
    
    voltage = analogRead(VOLTAGE_PIN) / 36.13;
    Serial.print("Battery voltage: ");
    Serial.print(voltage);
    Serial.println("V");

    if (voltage < infoVoltage) {
      if (voltage < warningVoltage) {
        warningVoltageMessage();
        Serial.println("Dangerously low voltage, shut off now!");
        voltageCheckInterval = 15L*1000;
      }
      else {
        infoVoltageMessage();
        Serial.println("Low voltage!");
        voltageCheckInterval = 60L*1000;
      }
    }

    lastCheckedVoltage = millis();
  }
}

void infoVoltageMessage() {
  analogWrite(LED_PIN, 0);
  delay(20);
  analogWrite(LED_PIN, actualPower);
}

void warningVoltageMessage() {
  analogWrite(LED_PIN, 0);
  delay(250);
  analogWrite(LED_PIN, 255);
  delay(20);
  analogWrite(LED_PIN, 0);
  delay(250);
  analogWrite(LED_PIN, actualPower);
}

