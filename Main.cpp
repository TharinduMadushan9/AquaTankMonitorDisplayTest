#include <SPI.h>
#include <TFT_eSPI.h>
#include <SPIFFS.h>

#define MOTOR1_PIN  13  // Drain pump
#define MOTOR2_PIN  14// Fill pump
#define SENSOR_BOTTOM_PIN  26  // Bottom-level drain sensor
#define SENSOR_TOP_PIN     27  // Top-level fill sensor


#define REPEAT_CAL false

TFT_eSPI tft = TFT_eSPI();
TFT_eSPI_Button btnATO, btnAWC, btnManual, btnSet, btnPlus, btnMinus, btnSave, btnStop;

bool atoOn = true;
bool awcAuto = true;
bool setMode = false;

uint16_t totalHours = 1;
uint8_t days = totalHours / 24;
uint8_t hours = totalHours % 24;

// Function prototypes
void drawStaticUI();
void drawTimeDisplay();
void updateStatus();
void drawButtons();
void drawSetButtons();
void touch_calibrate();

void setup() {
  Serial.begin(115200);
  SPIFFS.begin(true);

  pinMode(MOTOR1_PIN, OUTPUT);
  pinMode(MOTOR2_PIN, OUTPUT);
  pinMode(SENSOR_BOTTOM_PIN, INPUT_PULLUP);  // Active LOW
  pinMode(SENSOR_TOP_PIN, INPUT_PULLUP);     // Active LOW
  digitalWrite(MOTOR1_PIN, LOW);   
  digitalWrite(MOTOR2_PIN, LOW);

  tft.init();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);  
  touch_calibrate();

  drawStaticUI();     
  drawTimeDisplay();  
  updateStatus();    
  drawButtons();     
}

void loop() {
  uint16_t x, y;
  bool touched = tft.getTouch(&x, &y);  // FIXED: Added & before y
  static unsigned long lastPlus = 0;
  static unsigned long lastMinus = 0;

  if (!setMode) {
    // Handle ATO button
    btnATO.press(touched && btnATO.contains(x, y));
    if (btnATO.justReleased()) btnATO.drawButton();
    if (btnATO.justPressed()) {
      btnATO.drawButton(true);
      atoOn = !atoOn;
      updateStatus();
    }

    // Handle AWC button
    btnAWC.press(touched && btnAWC.contains(x, y));
    if (btnAWC.justReleased()) btnAWC.drawButton();
    if (btnAWC.justPressed()) {
      btnAWC.drawButton(true);
      awcAuto = !awcAuto;
      updateStatus();
    }

    // Handle Manual button
    btnManual.press(touched && btnManual.contains(x, y));
    if (btnManual.justReleased()) btnManual.drawButton();
    if (btnManual.justPressed()) {
      btnManual.drawButton(true);
      Serial.println("Manual Water Change Triggered!");
      performWaterChange();
    }

    // Handle Set button
    btnSet.press(touched && btnSet.contains(x, y));
    if (btnSet.justReleased()) btnSet.drawButton();
    if (btnSet.justPressed()) {
      btnSet.drawButton(true);
      setMode = true;
      tft.fillScreen(TFT_BLACK);  // Clear to black before redraw
      drawStaticUI();
      drawTimeDisplay();
      drawSetButtons();
    }
  } else {
    // Handle Plus button
    btnPlus.press(touched && btnPlus.contains(x, y));
    if (btnPlus.justReleased()) btnPlus.drawButton();
    if (btnPlus.justPressed()) {
      btnPlus.drawButton(true);
      totalHours++;
      days = totalHours / 24;
      hours = totalHours % 24;
      drawTimeDisplay();
      lastPlus = millis();
    }
    if (btnPlus.isPressed() && millis() - lastPlus > 100) {
      totalHours++;
      days = totalHours / 24;
      hours = totalHours % 24;
      drawTimeDisplay();
      lastPlus = millis();
    }

    // Handle Minus button
    btnMinus.press(touched && btnMinus.contains(x, y));
    if (btnMinus.justReleased()) btnMinus.drawButton();
    if (btnMinus.justPressed()) {
      btnMinus.drawButton(true);
      if (totalHours > 0) totalHours--;
      days = totalHours / 24;
      hours = totalHours % 24;
      drawTimeDisplay();
      lastMinus = millis();
    }
    if (btnMinus.isPressed() && millis() - lastMinus > 100) {
      if (totalHours > 0) totalHours--;
      days = totalHours / 24;
      hours = totalHours % 24;
      drawTimeDisplay();
      lastMinus = millis();
    }

    // Handle Save button
    btnSave.press(touched && btnSave.contains(x, y));
    if (btnSave.justReleased()) btnSave.drawButton();
    if (btnSave.justPressed()) {
      btnSave.drawButton(true);
      setMode = false;
      tft.fillScreen(TFT_BLACK);  // Clear to black before redraw
      drawStaticUI();
      drawTimeDisplay();
      updateStatus();
      drawButtons();
    }
  }
}

void drawStaticUI() {
  // Draw static text elements with black background
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(3);
  tft.setCursor(10, 5);  tft.print("ATO");
  tft.setCursor(140, 5); tft.print("AWC");
}

void drawTimeDisplay() {
  // Clear only the time display area with black
  tft.fillRect(25, 70, 200, 50, TFT_BLACK);
  
  // Draw time values
  int x = 25, y = 70;
  tft.setTextColor(TFT_WHITE, TFT_BLACK);  // Ensure black background
  tft.setTextSize(6);
  tft.setCursor(x, y); tft.printf("%02d", days);
  x += 70;

  tft.setTextSize(2);
  tft.setCursor(x, y + 30); tft.print("D");
  x += 10;

  tft.setTextSize(6);
  tft.setCursor(x, y); tft.print(":");
  x += 30;

  tft.setCursor(x, y); tft.printf("%02d", hours);
  x += 70;

  tft.setTextSize(2);
  tft.setCursor(x, y + 30); tft.print("Hr");
}

void updateStatus() {
  // Update status indicators with black background
  tft.fillRect(70, 10, 40, 15, TFT_BLACK);   // Clear ATO status area
  tft.fillRect(200, 10, 50, 15, TFT_BLACK);  // Clear AWC status area

  tft.setTextSize(1);
  tft.setTextColor(atoOn ? TFT_GREENYELLOW : TFT_RED, TFT_BLACK);
  tft.setCursor(70, 10); tft.print(atoOn ? "ON" : "OFF");

  tft.setTextColor(awcAuto ? TFT_GREENYELLOW : TFT_RED, TFT_BLACK);
  tft.setCursor(200, 10); tft.print(awcAuto ? "AUTO" : "MANUAL");
}

void drawButtons() {
  // Initialize buttons with black background
  btnATO.initButton(&tft, 50, 200, 60, 40, TFT_CYAN, TFT_DARKGREY, TFT_BLACK, "ATO", 2);
  btnAWC.initButton(&tft, 120, 200, 60, 40, TFT_CYAN, TFT_DARKGREY, TFT_BLACK, "AWC", 2);
  btnSet.initButton(&tft, 190, 200, 60, 40, TFT_GREENYELLOW, TFT_DARKGREEN, TFT_BLACK, "SET", 2);
  btnManual.initButton(&tft, 60, 260, 90, 40, TFT_CYAN, TFT_DARKGREY, TFT_BLACK, "Manual", 2);
  btnStop.initButton(&tft, 195, 260, 60, 40, TFT_WHITE, TFT_RED, TFT_WHITE, "STOP", 2);

  // Draw buttons (will fill their area with background color)
  btnATO.drawButton();
  btnAWC.drawButton();
  btnSet.drawButton();
  btnManual.drawButton();
  btnStop.drawButton();
}

void drawSetButtons() {
  // Initialize set buttons with black background
  btnPlus.initButton(&tft, 40, 150, 50, 40, TFT_CYAN, TFT_DARKGREY, TFT_BLACK, "+", 2);
  btnSave.initButton(&tft, 120, 150, 80, 40, TFT_GREENYELLOW, TFT_DARKGREEN, TFT_BLACK, "SAVE", 2);
  btnMinus.initButton(&tft, 200, 150, 50, 40, TFT_CYAN, TFT_DARKGREY, TFT_BLACK, "-", 2);

  // Draw buttons (will fill their area with background color)
  btnPlus.drawButton();
  btnSave.drawButton();
  btnMinus.drawButton();
}

void performWaterChange() {
  Serial.println("Starting water change...");

  // === STEP 1: DRAIN ===
  digitalWrite(MOTOR1_PIN, HIGH);
  Serial.println("Motor1 ON: Draining...");

  // Wait until sensorBottom goes HIGH (tank is empty)
  while (digitalRead(SENSOR_BOTTOM_PIN) == LOW) {
    delay(100);  // Check every 100ms
  }

  digitalWrite(MOTOR1_PIN, LOW);
  Serial.println("Drain complete. Motor1 OFF.");

  delay(500);  // Small gap before refill

  // === STEP 2: REFILL ===
  digitalWrite(MOTOR2_PIN, HIGH);
  Serial.println("Motor2 ON: Refilling...");

  // Wait until sensorTop goes HIGH (tank is full)
  while (digitalRead(SENSOR_TOP_PIN) == LOW) {
    delay(100);  // Check every 100ms
  }

  digitalWrite(MOTOR2_PIN, LOW);
  Serial.println("Refill complete. Motor2 OFF.");
}



void touch_calibrate() {
  uint16_t calData[5];
  bool calDataOK = false;

  if (SPIFFS.exists(CALIBRATION_FILE)) {
    fs::File f = SPIFFS.open(CALIBRATION_FILE, "r");
    if (f.readBytes((char *)calData, 14) == 14) calDataOK = true;
    f.close();
  }

  if (calDataOK && !REPEAT_CAL) {
    tft.setTouch(calData);
  } else {
    tft.fillScreen(TFT_BLACK);  // Use black background for calibration
    tft.setCursor(20, 0);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(2);
    tft.println("Touch corners as directed");
    tft.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);
    fs::File f = SPIFFS.open(CALIBRATION_FILE, "w");
    if (f) {
      f.write((const unsigned char *)calData, 14);
      f.close();
    }
  }
}
