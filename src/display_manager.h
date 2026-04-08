#pragma once

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

class DisplayManager {
  private:
  Adafruit_SSD1306 display;

  public:
  DisplayManager();

  bool begin();
  Adafruit_SSD1306& getDisplay();
};