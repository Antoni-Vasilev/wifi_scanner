#include "battery_app.h"
#include "../config.h"
#include <Adafruit_SSD1306.h>

extern void goBackApp();

BatteryApp::BatteryApp(DisplayManager* dm)
  : displayManager(dm),
    redraw(true),
    lastRenderedPercent(-1),
    lastRenderedCharging(false) {}

void BatteryApp::onEnter() {
  lastRenderedPercent = -1;
  lastRenderedCharging = !displayManager->isCharging();
  redraw = true;
}

void BatteryApp::forceRedraw() {
  int currentPercent   = displayManager->getBatteryPercent();
  bool currentCharging = displayManager->isCharging();

  if (currentPercent != lastRenderedPercent ||
      currentCharging != lastRenderedCharging) {
    redraw = true;
  }
}

void BatteryApp::handleInput(InputEvent event) {
  if (event == EVENT_LEFT) {
    goBackApp();
  }
}

void BatteryApp::render() {
  Adafruit_SSD1306& display = displayManager->getDisplay();

  int percent  = displayManager->getBatteryPercent();
  int mv       = displayManager->getLastMeasuredMV();
  bool charging = displayManager->isCharging();

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  display.setCursor(0, 0);
  display.print("Battery");
  display.drawLine(0, 10, 127, 10, WHITE);

  const int bx = 10;
  const int by = 14;
  const int bw = 100;
  const int bh = 20;

  display.drawRect(bx, by, bw, bh, WHITE);
  display.fillRect(bx + bw, by + 6, 4, 8, WHITE);

  int fillW = (percent * (bw - 2)) / 100;
  if (fillW > 0) {
    display.fillRect(bx + 1, by + 1, fillW, bh - 2, WHITE);
  }

  char pctStr[5];
  snprintf(pctStr, sizeof(pctStr), "%d%%", percent);
  int pctX = bx + (bw / 2) - (strlen(pctStr) * 6 / 2);
  int pctY = by + (bh / 2) - 4;

  display.setTextColor(percent > 50 ? BLACK : WHITE);
  display.setCursor(pctX, pctY);
  display.print(pctStr);
  display.setTextColor(WHITE);

  display.setCursor(0, 38);
  display.print("Voltage: ");
  display.print(mv);
  display.println(" mV");

  display.print("Status:  ");
  display.println(charging ? "Charging" : "Discharging");

  if (charging) {
    const int sx = 110;
    const int sy = 47;
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

  display.setCursor(0, 57);
  display.print("LEFT = Back");

  // БЕЗ display.display() — извиква се от AppManager

  lastRenderedPercent  = percent;
  lastRenderedCharging = charging;
}

bool BatteryApp::needsRedraw() const { return redraw; }
void BatteryApp::clearRedrawFlag() { redraw = false; }