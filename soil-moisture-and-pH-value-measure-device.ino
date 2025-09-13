#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Button pins
const int buttonNav = 2;   // Navigation button
const int buttonSelect = 3; // Select / Back button

// Sensor pins
const int soilPin = A0;   // Capacitive Soil Moisture Sensor
const int phPin = A1;     // PH4502C Module

// Menu variables
int menuIndex = 0;
const int menuLength = 2;
String menuItems[menuLength] = {"Measure Mode", "Calibration"};

bool inMeasureMode = false;

// For button handling
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 200; // ms
bool lastNavState = HIGH;
bool lastSelectState = HIGH;

unsigned long selectPressTime = 0;
bool selectPressed = false;

void setup() {
  pinMode(buttonNav, INPUT_PULLUP);
  pinMode(buttonSelect, INPUT_PULLUP);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println("Soil & PH Meter");
  display.display();
  delay(2000);
}

void loop() {
  handleButtons();

  if (inMeasureMode) {
    showMeasureMode();
  } else {
    showMenu();
  }
}

void handleButtons() {
  bool navState = digitalRead(buttonNav);
  bool selectState = digitalRead(buttonSelect);

  // Navigation button
  if (navState == LOW && lastNavState == HIGH && millis() - lastDebounceTime > debounceDelay) {
    menuIndex = (menuIndex + 1) % menuLength;
    lastDebounceTime = millis();
  }
  lastNavState = navState;

  // Select button
  if (selectState == LOW && lastSelectState == HIGH) {
    selectPressTime = millis();
    selectPressed = true;
  }

  if (selectState == HIGH && lastSelectState == LOW) {
    if (selectPressed && (millis() - selectPressTime < 1000)) {
      if (!inMeasureMode) {
        if (menuIndex == 0) {
          inMeasureMode = true; // Enter Measure Mode
        }
        else if (menuIndex == 1) {
          calibrationMode();
        }
      }
    }
    selectPressed = false;
  }

  // Long press for back
  if (selectPressed && (millis() - selectPressTime > 3000)) {
    inMeasureMode = false;
    selectPressed = false;
  }

  lastSelectState = selectState;
}

void showMenu() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Menu:");

  for (int i = 0; i < menuLength; i++) {
    if (i == menuIndex) display.print("> ");
    else display.print("  ");
    display.println(menuItems[i]);
  }

  display.display();
}

void showMeasureMode() {
  int soilValue = analogRead(soilPin);
  float soilPercent = map(soilValue, 1023, 200, 0, 100); // adjust mapping

  int phValue = analogRead(phPin);
  float voltage = phValue * (5.0 / 1023.0);
  float ph = 7 + ((2.5 - voltage) / 0.18); // rough PH calc

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Measure Mode:");
  display.print("Soil: ");
  display.print(soilPercent);
  display.println("%");
  display.print("pH: ");
  display.println(ph, 2);
  display.display();

  delay(500);
}

void calibrationMode() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Calibration Mode");
  display.println("Place in solution");
  display.display();
  delay(3000);
}
