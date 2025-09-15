#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// === OLED setup ===
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// === Pins ===
const int soilPin = A0;   // Soil sensor analog pin
const int phPin   = A1;   // pH sensor analog pin
const int buttonPin1 = 2; // Hold/Resume + Stop measuring
const int buttonPin2 = 3; // Countdown + average button

// === Calibration Defaults ===
int soilDry = 690;   // Dry soil (~0%)
int soilWet = 470;   // Wet soil (~100%)
float cal4 = 3.88;   // Voltage at pH 4.01
float cal7 = 3.08;   // Voltage at pH 7.00

// === Variables ===
bool hold = false;
int lastButton1State = HIGH;
int lastButton2State = HIGH;

bool countdownMode = false;
bool showAverage = false;

unsigned long startTime;
const int countdownDuration = 30; // seconds

// For averaging
long soilSum = 0;
long phSum = 0;
int sampleCount = 0;
float avgSoil = 0;
float avgPH = 0;

// Last (frozen) readings
int lastSoilRaw = 0;
int lastSoilPercent = 0;
int lastPhRaw = 0;
float lastPhVoltage = 0;
float lastPHValue = 0;

void setup() {
  pinMode(buttonPin1, INPUT_PULLUP);
  pinMode(buttonPin2, INPUT_PULLUP);
  Serial.begin(9600);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }

  // Show startup briefly and take an initial reading to seed frozen values
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(28, 7);
  display.println("Soil Moisture");
  display.setCursor(62, 21);
  display.println("&");
  display.setCursor(42, 35);
  display.println("pH Meter");
  display.setCursor(0, 55);
  display.println("by: JJay Technologies");
  display.display();
  delay(1500);

  // initial reading to populate last* variables
  performSensorRead(&lastSoilRaw, &lastSoilPercent, &lastPhRaw, &lastPhVoltage, &lastPHValue);
}

void loop() {
  int button1State = digitalRead(buttonPin1);
  int button2State = digitalRead(buttonPin2);

  // === Button 1 pressed ===
  if (button1State == LOW && lastButton1State == HIGH) {
    if (countdownMode) {
      // Stop countdown → back to live readings
      countdownMode = false;
      showAverage = false;
      hold = false;
    } else if (showAverage) {
      // Exit average screen → back to live readings
      showAverage = false;
    } else {
      // Toggle hold only in live mode
      hold = !hold;
      if (!hold) {
        // If un-holding, update last readings immediately so display continues from fresh values
        performSensorRead(&lastSoilRaw, &lastSoilPercent, &lastPhRaw, &lastPhVoltage, &lastPHValue);
      }
    }
    delay(200); // simple debounce
  }
  lastButton1State = button1State;

  // === Button 2 pressed ===
  if (button2State == LOW && lastButton2State == HIGH) {
    if (!countdownMode && !showAverage) {
      countdownMode = true;
      startTime = millis();
      soilSum = 0;
      phSum = 0;
      sampleCount = 0;
    } else if (showAverage) {
      // hide averages and go back to live
      showAverage = false;
    }
    delay(200);
  }
  lastButton2State = button2State;

  // === Live readings (or HOLD) ===
  if (!countdownMode && !showAverage) {
    displayReadings(); // it will respect the 'hold' flag internally
  }

  // === Countdown Mode ===
  if (countdownMode) {
    unsigned long elapsed = (millis() - startTime) / 1000;
    int remaining = countdownDuration - elapsed;

    if (remaining >= 0) {
      // Take readings during countdown (always sample during countdown)
      int soilRaw;
      int soilPercent;
      int phRaw;
      float phVoltage;
      float pHValue;
      performSensorRead(&soilRaw, &soilPercent, &phRaw, &phVoltage, &pHValue);

      // accumulate averages (use percent & pH float)
      soilSum += soilPercent;
      phSum += pHValue;
      sampleCount++;

      // --- Draw measuring screen with icons and progress bar ---
      display.clearDisplay();

      // Water icon + soil label (icon at x=0,y=0)
      drawWaterIcon(0, 0);
      display.setCursor(20, 0);
      display.setTextSize(1);
      display.println("Soil Moisture:");

      /*display.setCursor(20, 12);
      display.print("Raw: ");
      display.print(soilRaw);*/

      display.setCursor(20, 12);
      display.print("Level: ");
      display.print(soilPercent);
      display.println("%");

      // Flask icon + pH label (icon at x=0,y=36)
      drawFlaskIcon(0, 30);
      display.setCursor(20, 30);
      display.println("pH Sensor:");

      display.setCursor(20, 42);
      display.print("pH: ");
      display.print(pHValue, 2);

      // Progress bar at bottom
      drawProgressBar(elapsed);

      display.display();

      // small delay to control sample rate (~2 samples/sec)
      delay(500);
    } else {
      // compute and show averages permanently
      if (sampleCount > 0) {
        avgSoil = (float)soilSum / sampleCount;
        avgPH = (float)phSum / sampleCount;
      } else {
        avgSoil = 0;
        avgPH = 0;
      }

      Serial.println("=== Average Results ===");
      Serial.print("Soil Moisture: "); Serial.print(avgSoil, 1); Serial.println("%");
      Serial.print("pH: "); Serial.println(avgPH, 2);

      countdownMode = false;
      showAverage = true;
    }
  }

  // === Show Average Mode (persistent until button2 or button1 pressed) ===
  if (showAverage) {
    display.clearDisplay();

    // icons + labels
    drawWaterIcon(0, 16);
    display.setCursor(20, 0);
    display.setTextSize(1);
    display.println("Average (30s)");

    display.setCursor(20, 20);
    display.print("Moisture: ");
    display.print(avgSoil, 1);
    display.println("%");

    drawFlaskIcon(0, 36);
    display.setCursor(20, 41);
    display.print("pH value: ");
    display.print(avgPH, 2);

    display.display();
  }
}

// ========== Helper: perform sensor read (and update last* if caller provides last pointers) ==========
void performSensorRead(int *soilRawPtr, int *soilPercentPtr, int *phRawPtr, float *phVoltPtr, float *pHPtr) {
  int soilRaw = analogRead(soilPin);
  int soilPercent = map(soilRaw, soilDry, soilWet, 0, 100);
  soilPercent = constrain(soilPercent, 0, 100);

  int phRaw = analogRead(phPin) - 170;
  float phVoltage = phRaw * (5.0 / 1023.0);
  float slope = (7.0 - 4.0) / (cal7 - cal4);
  float intercept = 7.0 - slope * cal7;
  float pHValue = slope * phVoltage + intercept;

  // return via pointers if provided
  if (soilRawPtr) *soilRawPtr = soilRaw;
  if (soilPercentPtr) *soilPercentPtr = soilPercent;
  if (phRawPtr) *phRawPtr = phRaw;
  if (phVoltPtr) *phVoltPtr = phVoltage;
  if (pHPtr) *pHPtr = pHValue;

  // update last* snapshot only when in live (not hold) — caller controls that
  if (!hold) {
    lastSoilRaw = soilRaw;
    lastSoilPercent = soilPercent;
    lastPhRaw = phRaw;
    lastPhVoltage = phVoltage;
    lastPHValue = pHValue;
  }
}

// ========== Draw live / hold display (uses last* values when hold is true) ==========
void displayReadings() {
  // If not hold, update last readings right away
  if (!hold) {
    performSensorRead(NULL, NULL, NULL, NULL, NULL); // updates last* variables
  }
  // Draw using last* values (frozen when hold==true)
  display.clearDisplay();
  display.setTextSize(1);

  // Water icon + soil label
  drawWaterIcon(0, 0);
  display.setCursor(20, 0);
  display.println("Soil Moisture:");

  /*display.setCursor(20, 12);
  display.print("Raw: ");
  display.print(lastSoilRaw);*/

  display.setCursor(20, 12);
  display.print("Level: ");
  display.print(lastSoilPercent);
  display.println("%");

  // Flask icon + pH label
  drawFlaskIcon(0, 30);
  display.setCursor(20, 30);
  display.println("pH Sensor:");

  display.setCursor(20, 42);
  display.print("pH: ");
  display.print(lastPHValue, 2);

  // HOLD indicator (top-right)
  if (hold) {
    display.setCursor(104, 0);
    display.setTextSize(1);
    display.println("HOLD");
    // Optionally print to serial:
    Serial.println("=== HOLD MODE ===");
  }

  display.display();

  // Serial output for live readings:
  Serial.print("Soil Raw: "); Serial.print(lastSoilRaw);
  Serial.print(" | Moisture: "); Serial.print(lastSoilPercent); Serial.println("%");
  Serial.print("pH Raw: "); Serial.print(lastPhRaw);
  Serial.print(" | Voltage: "); Serial.print(lastPhVoltage, 2);
  Serial.print(" V | pH: "); Serial.println(lastPHValue, 2);

  delay(500); // preserve sample rate
}

// ========== Draw a simple water-drop icon using primitives ==========
void drawWaterIcon(int x, int y) {
  // small droplet: a rounded head + triangular tail
  display.fillCircle(x + 6, y + 4, 3, SSD1306_WHITE);            // top round part
  display.fillTriangle(x + 3, y + 6, x + 9, y + 6, x + 6, y + 11, SSD1306_WHITE); // tail
  // hollow center to make it look nicer (optional)
  display.drawCircle(x + 6, y + 4, 3, SSD1306_WHITE);
}

// ========== Draw a small flask icon with outline and 'liquid' level ==========
void drawFlaskIcon(int x, int y) {
  // neck
  display.drawRect(x + 4, y, 4, 4, SSD1306_WHITE);
  // body
  display.drawRect(x + 1, y + 4, 10, 10, SSD1306_WHITE);
  // liquid level (filled rectangle inside body)
  display.fillRect(x + 3, y + 10, 6, 3, SSD1306_WHITE); // a little "liquid"
}

// ========== Draw horizontal progress bar (bottom) ==========
void drawProgressBar(unsigned long elapsedSec) {
  int barY = 56;
  int barH = 7;
  // border
  display.drawRect(0, barY, SCREEN_WIDTH, barH, SSD1306_WHITE);
  // filled portion (elapsed -> width)
  int fillW = map(constrain((int)elapsedSec, 0, countdownDuration), 0, countdownDuration, 0, SCREEN_WIDTH);
  if (fillW > 0) display.fillRect(1, barY + 1, fillW - 1, barH - 2, SSD1306_WHITE);
}
