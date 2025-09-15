#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// === OLED setup ===
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// === Pins ===
const int soilPin = A0;   // Soil sensor analog pin
const int phPin   = A1;   // pH sensor analog pin
const int buttonPin1 = 2; // Hold/Resume button
const int buttonPin2 = 3; // Countdown + average button

// === Calibration Defaults ===
// Soil Moisture v2.0 typical values
int soilDry = 690;   // Dry soil (~0%)
int soilWet = 470;   // Wet soil (~100%)

// PH4502C calibration (using pH 4 and 7 buffer solutions)
float cal4 = 3.88;   // Voltage at pH 4.01
float cal7 = 3.08;   // Voltage at pH 7.00

// === Variables ===
bool hold = false;         
int lastButton1State = HIGH;
int lastButton2State = HIGH;

bool countdownMode = false;
bool showAverage = false;

unsigned long startTime;
int countdownDuration = 30; // seconds

// For averaging
long soilSum = 0;
long phSum = 0;
int sampleCount = 0;
float avgSoil = 0;
float avgPH = 0;

void setup() {
  pinMode(buttonPin1, INPUT_PULLUP);
  pinMode(buttonPin2, INPUT_PULLUP);
  Serial.begin(9600);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 20);
  display.println("Soil & pH Meter");
  display.display();
  delay(1500);
}

void loop() {
  int button1State = digitalRead(buttonPin1);
  int button2State = digitalRead(buttonPin2);

  // === Button 1: Toggle Hold ===
  if (button1State == LOW && lastButton1State == HIGH && !countdownMode && !showAverage) {
    hold = !hold;
    delay(200); // debounce
  }
  lastButton1State = button1State;

  // === Button 2: Control Countdown/Average ===
  if (button2State == LOW && lastButton2State == HIGH) {
    if (!countdownMode && !showAverage) {
      // Start countdown
      countdownMode = true;
      startTime = millis();
      soilSum = 0;
      phSum = 0;
      sampleCount = 0;
    } else if (showAverage) {
      // Exit average screen
      showAverage = false;
    }
    delay(200); // debounce
  }
  lastButton2State = button2State;

  // === Normal Mode ===
  if (!hold && !countdownMode && !showAverage) {
    displayReadings();
  }

  // === Countdown Mode ===
  if (countdownMode) {
    unsigned long elapsed = (millis() - startTime) / 1000;
    int remaining = countdownDuration - elapsed;

    if (remaining >= 0) {
      // Take readings during countdown
      int soilRaw = analogRead(soilPin);
      int soilPercent = map(soilRaw, soilDry, soilWet, 0, 100);
      soilPercent = constrain(soilPercent, 0, 100);

      int phRaw = analogRead(phPin) - 170;
      float phVoltage = phRaw * (5.0 / 1023.0);
      float slope = (7.0 - 4.0) / (cal7 - cal4);
      float intercept = 7.0 - slope * cal7;
      float pHValue = slope * phVoltage + intercept;

      soilSum += soilPercent;
      phSum += pHValue;
      sampleCount++;

      // Show countdown
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("Measuring...");
      display.setCursor(0, 16);
      display.print("Time Left: ");
      display.print(remaining);
      display.println("s");
      display.display();

      delay(500); // update ~2 times per second
    } 
    else {
      // Store averages
      avgSoil = (float)soilSum / sampleCount;
      avgPH = (float)phSum / sampleCount;

      Serial.println("=== Average Results ===");
      Serial.print("Soil Moisture: "); Serial.print(avgSoil, 1); Serial.println("%");
      Serial.print("pH: "); Serial.println(avgPH, 2);

      countdownMode = false;
      showAverage = true;
    }
  }

  // === Show Average Mode ===
  if (showAverage) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Average Results:");
    display.setCursor(0, 16);
    display.print("Soil: ");
    display.print(avgSoil, 1);
    display.println("%");

    display.setCursor(0, 32);
    display.print("pH: ");
    display.print(avgPH, 2);
    display.display();
  }
}

void displayReadings() {
  // === Soil Moisture ===
  int soilRaw = analogRead(soilPin);
  int soilPercent = map(soilRaw, soilDry, soilWet, 0, 100);
  soilPercent = constrain(soilPercent, 0, 100);

  // === pH Reading ===
  int phRaw = analogRead(phPin) - 170;
  float phVoltage = phRaw * (5.0 / 1023.0);
  float slope = (7.0 - 4.0) / (cal7 - cal4);
  float intercept = 7.0 - slope * cal7;
  float pHValue = slope * phVoltage + intercept;

  // === OLED Display ===
  display.clearDisplay();
  display.setTextSize(1);

  display.setCursor(0, 0);
  display.println("Soil Moisture:");

  display.setCursor(0, 12);
  display.print("Raw: ");
  display.print(soilRaw);

  display.setCursor(0, 24);
  display.print("Level: ");
  display.print(soilPercent);
  display.println("%");

  display.setCursor(0, 36);
  display.println("pH Sensor:");

  display.setCursor(0, 48);
  display.print("Raw: ");
  display.print(phRaw);

  display.setCursor(64, 48);
  display.print("pH: ");
  display.print(pHValue, 2);

  display.display();

  // === Serial Output ===
  Serial.print("Soil Raw: "); Serial.print(soilRaw);
  Serial.print(" | Moisture: "); Serial.print(soilPercent); Serial.println("%");

  Serial.print("pH Raw: "); Serial.print(phRaw);
  Serial.print(" | Voltage: "); Serial.print(phVoltage, 2);
  Serial.print(" V | pH: "); Serial.println(pHValue, 2);

  delay(500);
}
