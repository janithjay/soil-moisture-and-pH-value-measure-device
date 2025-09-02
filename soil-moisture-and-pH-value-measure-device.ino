#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>

// ---------------- LCD Setup ----------------
LiquidCrystal_I2C lcd(0x27, 16, 2); // Change address if needed

// ---------------- Pin Setup ----------------
const int soilPin = A0;
const int phPin   = A1;

// Buttons (active LOW with pull-ups)
const int btnUp    = 2;
const int btnDown  = 3;
const int btnSelect= 4;

// ---------------- Calibration Storage ----------------
float soilDry = 300;   // Default values (update during calibration)
float soilWet = 700;
float phLowRef = 3.0, phLowVal = 1500;
float phHighRef = 7.0, phHighVal = 2000;

// Menu variables
int menuIndex = 0;
int menuCount = 3; // Main menu items
bool inMenu = true;

// ---------------- Setup ----------------
void setup() {
  lcd.init();
  lcd.backlight();

  pinMode(btnUp, INPUT_PULLUP);
  pinMode(btnDown, INPUT_PULLUP);
  pinMode(btnSelect, INPUT_PULLUP);

  // Load calibration from EEPROM
  EEPROM.get(0, soilDry);
  EEPROM.get(4, soilWet);
  EEPROM.get(8, phLowRef);
  EEPROM.get(12, phLowVal);
  EEPROM.get(16, phHighRef);
  EEPROM.get(20, phHighVal);

  lcd.setCursor(0, 0);
  lcd.print("Soil & pH Meter");
  lcd.setCursor(0, 1);
  lcd.print("Initializing...");
  delay(2000);
}

// ---------------- Button Handling ----------------
bool isPressed(int pin) {
  return digitalRead(pin) == LOW;
}

// ---------------- Menu System ----------------
void loop() {
  if (inMenu) {
    showMenu();
    handleMenuInput();
  } else {
    measureAndDisplay();
  }
}

void showMenu() {
  lcd.clear();
  lcd.setCursor(0, 0);
  if (menuIndex == 0) lcd.print("> Measure");
  else lcd.print("  Measure");

  lcd.setCursor(0, 1);
  if (menuIndex == 1) lcd.print("> Cal Soil");
  else if (menuIndex == 2) lcd.print("> Cal pH");
  else lcd.print("  Cal Soil");
}

void handleMenuInput() {
  if (isPressed(btnUp)) {
    menuIndex = (menuIndex - 1 + menuCount) % menuCount;
    delay(300);
  }
  if (isPressed(btnDown)) {
    menuIndex = (menuIndex + 1) % menuCount;
    delay(300);
  }
  if (isPressed(btnSelect)) {
    delay(300);
    if (menuIndex == 0) inMenu = false; // Go to measurement
    else if (menuIndex == 1) calibrateSoil();
    else if (menuIndex == 2) calibratePH();
  }
}

// ---------------- Soil Calibration ----------------
void calibrateSoil() {
  lcd.clear();
  lcd.print("Insert in DRY");
  lcd.setCursor(0,1); lcd.print("Press SELECT");
  while (!isPressed(btnSelect));
  soilDry = analogRead(soilPin);
  EEPROM.put(0, soilDry);

  lcd.clear();
  lcd.print("Insert in WET");
  lcd.setCursor(0,1); lcd.print("Press SELECT");
  while (!isPressed(btnSelect));
  soilWet = analogRead(soilPin);
  EEPROM.put(4, soilWet);

  lcd.clear();
  lcd.print("Soil Cal Saved");
  delay(2000);
}

// ---------------- pH Calibration ----------------
void calibratePH() {
  lcd.clear();
  lcd.print("Insert in pH 4");
  lcd.setCursor(0,1); lcd.print("Press SELECT");
  while (!isPressed(btnSelect));
  phLowVal = analogRead(phPin);
  phLowRef = 4.0;
  EEPROM.put(8, phLowRef);
  EEPROM.put(12, phLowVal);

  lcd.clear();
  lcd.print("Insert in pH 7");
  lcd.setCursor(0,1); lcd.print("Press SELECT");
  while (!isPressed(btnSelect));
  phHighVal = analogRead(phPin);
  phHighRef = 7.0;
  EEPROM.put(16, phHighRef);
  EEPROM.put(20, phHighVal);

  lcd.clear();
  lcd.print("pH Cal Saved");
  delay(2000);
}

// ---------------- Measurement Mode ----------------
void measureAndDisplay() {
  int rawSoil = analogRead(soilPin);
  int rawPH = analogRead(phPin);

  float soilPercent = map(rawSoil, soilDry, soilWet, 0, 100);
  if (soilPercent < 0) soilPercent = 0;
  if (soilPercent > 100) soilPercent = 100;

  float slope = (phHighRef - phLowRef) / (phHighVal - phLowVal);
  float phValue = phLowRef + slope * (rawPH - phLowVal);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Soil: ");
  lcd.print(soilPercent);
  lcd.print("%");

  lcd.setCursor(0, 1);
  lcd.print("pH: ");
  lcd.print(phValue, 2);

  delay(1000);

  if (isPressed(btnSelect)) { // Exit to menu
    inMenu = true;
    delay(300);
  }
}
