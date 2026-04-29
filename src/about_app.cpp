#include "about_app.h"
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <esp_system.h>

extern void goBackApp();

AboutApp::AboutApp(DisplayManager* dm)
  : displayManager(dm), redraw(true) {}

void AboutApp::onEnter() {
  redraw = true;
}

void AboutApp::handleInput(InputEvent event) {
  if (event == EVENT_LEFT) {
    goBackApp();
  }
}

void AboutApp::render() {
  Adafruit_SSD1306& display = displayManager->getDisplay();

  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);

  // Header
  display.setCursor(0, 0);
  display.print("About");
  display.drawLine(0, 10, 127, 10, WHITE);

  // MAC адрес на ESP32
  display.setCursor(0, 13);
  display.print("MAC: ");
  display.println(WiFi.macAddress());

  // Свободна RAM
  uint32_t freeRam = esp_get_free_heap_size() / 1024;
  display.print("RAM: ");
  display.print(freeRam);
  display.println(" KB free");

  // Uptime
  unsigned long totalSec = millis() / 1000;
  unsigned long hours    = totalSec / 3600;
  unsigned long mins     = (totalSec % 3600) / 60;
  unsigned long secs     = totalSec % 60;

  char uptimeStr[20];
  snprintf(uptimeStr, sizeof(uptimeStr), "%luh %02lum %02lus", hours, mins, secs);
  display.print("Up:  ");
  display.println(uptimeStr);

  // Версия
  display.print("Ver: v1.0");

  // Автори
  display.drawLine(0, 53, 127, 53, WHITE);
  display.setCursor(0, 56);
  display.print("Antoni & Radostin");

  // БЕЗ display.display() — извиква се от AppManager
}

bool AboutApp::needsRedraw() const { return redraw; }
void AboutApp::clearRedrawFlag() { redraw = false; }