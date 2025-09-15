#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// === OLED setup ===
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// === Pins ===
const int soilPin = A0;   // Soil sensor analog pin
const int phPin   = A1;   // pH sensor analog pin
const int buttonPin = 2;  // Push button pin

// === Calibration Defaults (No user calibration needed) ===
// Soil Moisture v2.0 typical values
int soilDry = 690;   // Dry soil (~0%)
int soilWet = 470;    // Wet soil (~100%)

// PH4502C calibration (using pH 4 and 7 buffer solutions)
float cal4 = 3.88;    // Voltage at pH 4.01
float cal7 = 3.08;    // Voltage at pH 7.00

// === Variables ===
bool hold = false;         
int lastButtonState = HIGH;

void setup() {
  pinMode(buttonPin, INPUT_PULLUP);
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
  int buttonState = digitalRead(buttonPin);

  // Toggle hold state
  if (buttonState == LOW && lastButtonState == HIGH) {
    hold = !hold;
    delay(200); // debounce
  }
  lastButtonState = buttonState;

  if (!hold) {
    // === Soil Moisture ===
    int soilRaw = analogRead(soilPin);
    int soilPercent = map(soilRaw, soilDry, soilWet, 0, 100);
    soilPercent = constrain(soilPercent, 0, 100);

    // === pH Reading (using pH4–pH7 calibration) ===
    int phRaw = analogRead(phPin) - 170;
    float phVoltage = phRaw * (5.0 / 1023.0);

    // Linear interpolation formula
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
  }
}
