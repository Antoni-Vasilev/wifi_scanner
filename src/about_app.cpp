#include "about_app.h"
#include <Adafruit_SSD1306.h>

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

  display.setCursor(0, 0);
  display.println("About");
  display.drawLine(0, 10, 127, 10, WHITE);

  display.setCursor(0, 16);
  display.println("ESP32-C3 UI Demo");
  display.println("Menu architecture");
  display.println("with multiple apps");
  display.println();
  display.println("LEFT = Back");

  // БЕЗ display.display() — извиква се от AppManager
}

bool AboutApp::needsRedraw() const { return redraw; }
void AboutApp::clearRedrawFlag() { redraw = false; }