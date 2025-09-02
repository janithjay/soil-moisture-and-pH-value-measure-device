#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <EEPROM.h>

// OLED setup
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Pin configuration
#define SOIL_PIN A0       // Capacitive soil moisture sensor
#define PH_PIN A1         // pH4502c analog output
#define BUTTON_MENU 2     // Menu button
#define BUTTON_SELECT 3   // Select button
#define BUTTON_PH4 4      // Calibrate pH 4 button
#define BUTTON_PH7 5      // Calibrate pH 7 button

// Variables for pH calibration
float ph4Voltage = 1.65;  // Default (to be updated after calibration)
float ph7Voltage = 2.5;   // Default (to be updated after calibration)
float slope = -1.0;
float intercept = 7.0;

// Menu state
int menuIndex = 0;  // 0 = Measure, 1 = Calibrate
int subMenuIndex = 0;
bool inSubMenu = false;

// Debounce
unsigned long lastButtonPress = 0;

// -------------------- FUNCTIONS --------------------
void loadCalibration() {
  EEPROM.get(0, ph4Voltage);
  EEPROM.get(sizeof(float), ph7Voltage);
  if (isnan(ph4Voltage) || isnan(ph7Voltage)) {
    ph4Voltage = 1.65;
    ph7Voltage = 2.5;
  }
  slope = (7.0 - 4.0) / (ph7Voltage - ph4Voltage);
  intercept = 7.0 - slope * ph7Voltage;
}

void saveCalibration() {
  EEPROM.put(0, ph4Voltage);
  EEPROM.put(sizeof(float), ph7Voltage);
}

// Read soil moisture (%)
int readSoilMoisture() {
  int value = analogRead(SOIL_PIN);
  int moisture = map(value, 600, 300, 0, 100); // Adjust mapping if needed
  if (moisture < 0) moisture = 0;
  if (moisture > 100) moisture = 100;
  return moisture;
}

// Read pH
float readPH() {
  int value = analogRead(PH_PIN);
  float voltage = value * (5.0 / 1023.0);
  float phValue = slope * voltage + intercept;
  return phValue;
}

// Draw main menu
void drawMenu() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.println("== Main Menu ==");

  display.setCursor(0, 16);
  if (menuIndex == 0) display.print("> ");
  display.println("Measure");

  display.setCursor(0, 32);
  if (menuIndex == 1) display.print("> ");
  display.println("Calibrate");

  display.display();
}

// Draw measure screen
void drawMeasure() {
  int moisture = readSoilMoisture();
  float phValue = readPH();

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("== Measurements ==");

  display.setCursor(0, 16);
  display.print("Moisture: ");
  display.print(moisture);
  display.println("%");

  display.setCursor(0, 32);
  display.print("pH: ");
  display.print(phValue, 2);

  display.display();
}

// Draw calibrate menu
void drawCalibrateMenu() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.println("== Calibrate ==");

  display.setCursor(0, 16);
  if (subMenuIndex == 0) display.print("> ");
  display.println("Soil Moisture");

  display.setCursor(0, 32);
  if (subMenuIndex == 1) display.print("> ");
  display.println("pH Sensor");

  display.display();
}

// -------------------- SETUP --------------------
void setup() {
  pinMode(BUTTON_MENU, INPUT_PULLUP);
  pinMode(BUTTON_SELECT, INPUT_PULLUP);
  pinMode(BUTTON_PH4, INPUT_PULLUP);
  pinMode(BUTTON_PH7, INPUT_PULLUP);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    for (;;); // Don't proceed if OLED init fails
  }
  display.clearDisplay();

  loadCalibration();
  drawMenu();
}

// -------------------- LOOP --------------------
void loop() {
  if (digitalRead(BUTTON_MENU) == LOW && millis() - lastButtonPress > 200) {
    lastButtonPress = millis();
    if (!inSubMenu) {
      menuIndex = (menuIndex + 1) % 2;
      drawMenu();
    } else {
      subMenuIndex = (subMenuIndex + 1) % 2;
      drawCalibrateMenu();
    }
  }

  if (digitalRead(BUTTON_SELECT) == LOW && millis() - lastButtonPress > 200) {
    lastButtonPress = millis();
    if (!inSubMenu) {
      if (menuIndex == 0) {
        drawMeasure();
        delay(2000);
        drawMenu();
      } else if (menuIndex == 1) {
        inSubMenu = true;
        drawCalibrateMenu();
      }
    } else {
      if (subMenuIndex == 0) {
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("Soil Moisture sensor\nneeds no calibration.");
        display.display();
        delay(2000);
        drawCalibrateMenu();
      } else if (subMenuIndex == 1) {
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("pH Calibration Mode");
        display.println("Press pH4 or pH7");
        display.display();

        unsigned long start = millis();
        while (millis() - start < 10000) { // 10s to calibrate
          if (digitalRead(BUTTON_PH4) == LOW) {
            int raw = analogRead(PH_PIN);
            ph4Voltage = raw * (5.0 / 1023.0);
            saveCalibration();
            loadCalibration();
            display.clearDisplay();
            display.setCursor(0, 0);
            display.println("pH 4 calibrated!");
            display.display();
            delay(2000);
            break;
          }
          if (digitalRead(BUTTON_PH7) == LOW) {
            int raw = analogRead(PH_PIN);
            ph7Voltage = raw * (5.0 / 1023.0);
            saveCalibration();
            loadCalibration();
            display.clearDisplay();
            display.setCursor(0, 0);
            display.println("pH 7 calibrated!");
            display.display();
            delay(2000);
            break;
          }
        }
        drawCalibrateMenu();
      }
    }
  }
}
