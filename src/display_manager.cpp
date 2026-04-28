#include "display_manager.h"
#include "../config.h"

DisplayManager::DisplayManager()
  : display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1),
    batteryPercent(0),
    lastMeasuredMV(0),
    lastBatRead(0) {}

bool DisplayManager::begin() {
  Wire.begin(I2C_SDA, I2C_SCL);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    return false;
  }

  pinMode(BAT_ADC_PIN, INPUT);

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setRotation(2);
  display.display();

  readBattery();

  return true;
}

Adafruit_SSD1306& DisplayManager::getDisplay() {
  return display;
}

// =========================
// Battery reading
// =========================

void DisplayManager::readBattery() {
  long sum = 0;
  for (int i = 0; i < 8; i++) {
    sum += analogRead(BAT_ADC_PIN);
    delay(2);
  }
  int raw = sum / 8;

  int millivolts = (raw * 3300 / 4095) * 2;
  lastMeasuredMV = millivolts;

  int percent = map(millivolts, BAT_MIN_MV, BAT_MAX_MV, 0, 100);
  if (percent > 100) percent = 100;
  if (percent < 0)   percent = 0;

  batteryPercent = percent;
}

void DisplayManager::updateBattery() {
  unsigned long now = millis();
  if (now - lastBatRead >= BAT_READ_INTERVAL_MS) {
    lastBatRead = now;
    readBattery();
  }
}

int DisplayManager::getBatteryPercent() const {
  return batteryPercent;
}

int DisplayManager::getLastMeasuredMV() const {
  return lastMeasuredMV;
}

bool DisplayManager::isCharging() const {
  return lastMeasuredMV > BAT_CHARGING_MV;
}

// =========================
// Battery overlay
// =========================

void DisplayManager::drawOverlay() {
  // Само рисува в буфера — display.display() се извиква от AppManager
  drawBatteryOverlay();
}

void DisplayManager::drawBatteryOverlay() {
  const int x = 112;
  const int y = 0;

  display.fillRect(x - 6, y, 22, 7, BLACK);

  display.drawRect(x, y, 13, 7, WHITE);
  display.fillRect(x + 13, y + 2, 2, 3, WHITE);

  int fillWidth = (batteryPercent * 11) / 100;
  if (fillWidth > 0) {
    display.fillRect(x + 1, y + 1, fillWidth, 5, WHITE);
  }

  if (isCharging()) {
    const int sx = x - 6;
    const int sy = y + 1;
    display.drawPixel(sx + 1, sy + 0, WHITE);
    display.drawPixel(sx + 3, sy + 0, WHITE);
    display.drawPixel(sx + 2, sy + 1, WHITE);
    display.drawPixel(sx + 0, sy + 2, WHITE);
    display.drawPixel(sx + 1, sy + 2, WHITE);
    display.drawPixel(sx + 2, sy + 2, WHITE);
    display.drawPixel(sx + 3, sy + 2, WHITE);
    display.drawPixel(sx + 4, sy + 2, WHITE);
    display.drawPixel(sx + 2, sy + 3, WHITE);
    display.drawPixel(sx + 1, sy + 4, WHITE);
    display.drawPixel(sx + 3, sy + 4, WHITE);
  }
}