#pragma once

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

class DisplayManager {
  private:
  Adafruit_SSD1306 display;

  int batteryPercent;
  int lastMeasuredMV;
  unsigned long lastBatRead;

  static const unsigned long BAT_READ_INTERVAL_MS = 100;

  void readBattery();
  void drawBatteryOverlay();

  public:
  DisplayManager();

  bool begin();
  Adafruit_SSD1306& getDisplay();

  void updateBattery();
  void drawOverlay(); // Рисува само в буфера — БЕЗ display.display()

  int getBatteryPercent() const;
  int getLastMeasuredMV() const;
  bool isCharging() const;
};