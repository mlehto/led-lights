#include <Bounce2.h>

#define RED 9
#define GREEN 10
#define BLUE 11
#define WHITE 6
#define VOLTAGE_PIN 14

#define BTN_COLOR_UP 7
#define BTN_COLOR_DOWN 8
long buttonDownDelay = 75L*64;
long buttonDownTime = 0;

#define BTN_COLORPOWER_UP 4
#define BTN_COLORPOWER_DOWN 5

#define BTN_POWER_UP 12
#define BTN_POWER_DOWN 15

int redValue = 0;
int greenValue = 0;
int blueValue = 0;
int whiteValue = 0;
int delayTime = 50;

int selectedColorIndex = 0;
int selectedColorPowerIndex = 0;
double colorPowers[] = {0, 0.125, 0.25, 0.5 , 1};
bool colorPowerCanBeChanged = true;

int selectedPowerIndex = 4;
double powers[] = {0, 1, 2, 4, 8, 16, 32, 64, 128, 255};
bool powerCanBeChanged = true;

Bounce colorUp = Bounce(); Bounce colorDown = Bounce(); 
Bounce colorPowerUp = Bounce(); Bounce colorPowerDown = Bounce();
Bounce powerUp = Bounce(); Bounce powerDown = Bounce();

unsigned long lastCheckedVoltage;
unsigned long voltageCheckInterval = 64 * 2L * 1000;
double voltage;
double infoVoltage = 3.6*6;
double warningVoltage = 3.4*6;

void setup() {
  pinMode(RED, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(BLUE, OUTPUT);
  pinMode(WHITE, OUTPUT); 
  allOff();
  Serial.begin(9600);

  pinMode(BTN_COLOR_UP, INPUT_PULLUP);
  colorUp.attach(BTN_COLOR_UP);
  colorUp.interval(5);
  pinMode(BTN_COLOR_DOWN, INPUT_PULLUP);
  colorDown.attach(BTN_COLOR_DOWN);
  colorDown.interval(5);
  
  pinMode(BTN_COLORPOWER_UP, INPUT_PULLUP);
  colorPowerUp.attach(BTN_COLORPOWER_UP);
  colorPowerUp.interval(5);
  pinMode(BTN_COLORPOWER_DOWN, INPUT_PULLUP);
  colorPowerDown.attach(BTN_COLORPOWER_DOWN);
  colorPowerDown.interval(5);

  pinMode(BTN_POWER_UP, INPUT_PULLUP);
  powerUp.attach(BTN_POWER_UP);
  powerUp.interval(5);
  pinMode(BTN_POWER_DOWN, INPUT_PULLUP);
  powerDown.attach(BTN_POWER_DOWN);
  powerDown.interval(5);

  pinMode(VOLTAGE_PIN, INPUT);

  TCCR0B = TCCR0B & B11111000 | B00000001;    // set timer 0 divisor to 1 = 62500.00 Hz
  TCCR1B = TCCR1B & B11111000 | B00000001;    // set timer 1 divisor to 1 = 31372.55 Hz
  TCCR2B = TCCR2B & B11111000 | B00000001;    // set timer 2 divisor to 1 = 31372.55 Hz
}

void loop() {
  //Check and update color
  colorUp.update();
  colorDown.update();
  int colorUpValue = colorUp.read();
  int colorDownValue = colorDown.read();
  
  if ((colorUpValue == LOW || colorDownValue == LOW) && buttonDownTime + buttonDownDelay < millis() && selectedColorPowerIndex > 0) {
    buttonDownTime = millis();
    if (colorUpValue == LOW) {
      selectedColorIndex += 3;
    }
    if (colorDownValue == LOW) {
      selectedColorIndex -= 3;
    }
    if (selectedColorIndex > 767) selectedColorIndex = 0;
    if (selectedColorIndex < 0) selectedColorIndex = 767;
  }

  if (selectedColorIndex < 256) {
    greenValue = selectedColorIndex;
    redValue = 255 - selectedColorIndex;
    blueValue = 0;
  }
  if (selectedColorIndex >= 256 && selectedColorIndex < 512) {
    blueValue = selectedColorIndex - 256;
    greenValue = 255 - selectedColorIndex + 256;
    redValue = 0;
  }
  if (selectedColorIndex >= 512) {
    redValue = selectedColorIndex - 512;
    blueValue = 255 - selectedColorIndex + 512;
    greenValue = 0;
  }

  //Check and update color power
  colorPowerUp.update();
  colorPowerDown.update();
  int colorPowerUpValue = colorPowerUp.read();
  int colorPowerDownValue = colorPowerDown.read();

  if (colorPowerUpValue == LOW && colorPowerCanBeChanged) {
    selectedColorPowerIndex++;
    if (selectedColorPowerIndex > 4) selectedColorPowerIndex = 4;
    colorPowerCanBeChanged = false;
  }
  if (colorPowerDownValue == LOW && colorPowerCanBeChanged) {
    selectedColorPowerIndex--;
    if (selectedColorPowerIndex < 0) selectedColorPowerIndex = 0;
    if (selectedColorPowerIndex == 0 && selectedPowerIndex == 0) selectedPowerIndex = 1;
    colorPowerCanBeChanged = false;
  }
  if (colorPowerUpValue == HIGH && colorPowerDownValue == HIGH) colorPowerCanBeChanged = true;

  //Check and update adlux power
  powerUp.update();
  powerDown.update();
  int powerUpValue = powerUp.read();
  int powerDownValue = powerDown.read();

  if (powerUpValue == LOW && powerCanBeChanged) {
    selectedPowerIndex++;
    if (selectedPowerIndex > 9) selectedPowerIndex = 9;
    powerCanBeChanged = false;
  }
  if (powerDownValue == LOW && powerCanBeChanged) {
    selectedPowerIndex--;
    if (selectedPowerIndex < 0) selectedPowerIndex = 0;
    if (selectedPowerIndex == 0 && selectedColorPowerIndex == 0) selectedColorPowerIndex = 1;
    powerCanBeChanged = false;
  }
  if (powerUpValue == HIGH && powerDownValue == HIGH) powerCanBeChanged = true;
  
  //Update outputs
  redValue = redValue * colorPowers[selectedColorPowerIndex];
  greenValue = greenValue * colorPowers[selectedColorPowerIndex];
  blueValue = blueValue * colorPowers[selectedColorPowerIndex];
  whiteValue = powers[selectedPowerIndex];
  
  analogWrite(RED, redValue);
  analogWrite(GREEN, greenValue);
  analogWrite(BLUE, blueValue);
  analogWrite(WHITE, whiteValue);

  //Voltage monitor
  if (lastCheckedVoltage + voltageCheckInterval < millis()) {
    voltageCheckInterval = 64*3*60L*1000;
    Serial.print("Power set to: W");
    Serial.print(whiteValue);
    Serial.print(" R");
    Serial.print(redValue);
    Serial.print(" G");
    Serial.print(greenValue);
    Serial.print(" B");
    Serial.println(blueValue);
    
    voltage = analogRead(VOLTAGE_PIN) / 36.1;
    Serial.print("Battery voltage: ");
    Serial.print(voltage);
    Serial.println("V");

    if (voltage < infoVoltage) {
      if (voltage < warningVoltage) {
        warningVoltageMessage();
        Serial.println("Dangerously low voltage, shut off now!");
        voltageCheckInterval = 64*15L*1000;
      }
      else {
        infoVoltageMessage();
        Serial.println("Low voltage!");
        voltageCheckInterval = 64*60L*1000;
      }
    }

    lastCheckedVoltage = millis();
  }
}

void infoVoltageMessage() {
  analogWrite(WHITE, 0);
  analogWrite(RED, 0);
  analogWrite(GREEN, 0);
  analogWrite(BLUE, 0);
  delay(20);
  analogWrite(RED, redValue);
  analogWrite(GREEN, greenValue);
  analogWrite(BLUE, blueValue);
  analogWrite(WHITE, whiteValue);
}

void warningVoltageMessage() {
  for (int i = 0 ; i < 3 ; i++) {
    analogWrite(RED, 0);
    delay(250);
    analogWrite(RED, 255);
    delay(20);
    analogWrite(RED, 0);
    delay(250);
    analogWrite(RED, redValue);
    
    analogWrite(WHITE, 0);
    delay(250);
    analogWrite(WHITE, 255);
    delay(20);
    analogWrite(WHITE, 0);
    delay(250);
    analogWrite(WHITE, whiteValue);
  }
}

void allOff() {
  analogWrite(RED, 0);
  analogWrite(GREEN, 0);
  analogWrite(BLUE, 0);
  analogWrite(WHITE, 0);
}

