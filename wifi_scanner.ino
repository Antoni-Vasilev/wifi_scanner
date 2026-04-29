#include <Arduino.h>
#include <esp_wifi.h>

#include "config.h"
#include "src/app_manager.h"
#include "src/input_manager.h"
#include "src/display_manager.h"
#include "src/menu_app.h"
#include "src/menu_types.h"
#include "src/about_app.h"
#include "src/scan_app.h"
#include "src/battery_app.h"
#include "src/keyboard_app.h"
#include "src/connect_app.h"
#include "src/speedtest_app.h"
#include "src/nvs_manager.h"
#include "src/saved_app.h"

// =========================
// Global managers
// =========================
DisplayManager displayManager;
InputManager   inputManager;
AppManager     appManager;
NVSManager     nvsManager;

// =========================
// App instances
// =========================
ScanApp      scanApp(&displayManager, &nvsManager);
AboutApp     aboutApp(&displayManager);
BatteryApp   batteryApp(&displayManager);
KeyboardApp  keyboardApp(&displayManager);
ConnectApp   connectApp(&displayManager, &nvsManager);
SpeedtestApp speedtestApp(&displayManager);
SavedApp     savedApp(&displayManager, &nvsManager);

// Forward declarations
void goBackApp();
void openAboutApp();
void openScanApp();
void openBatteryApp();
void openSavedApp();
void openKeyboardForNetwork(const char* ssid);
void connectToNetwork(const char* ssid, const char* password);
void openConnectApp(const char* ssid, const char* password);
void openSpeedtestApp();

// =========================
// Menu definitions
// =========================
extern Menu toolsMenu;
extern Menu mainMenu;

MenuItem toolsItems[] = {
  { "Scan",    MENU_ACTION, openScanApp,    nullptr },
  { "Saved",   MENU_ACTION, openSavedApp,   nullptr },
  { "Battery", MENU_ACTION, openBatteryApp, nullptr },
  { "Back",    MENU_BACK,   nullptr,        nullptr }
};

MenuItem mainItems[] = {
  { "Tools", MENU_SUBMENU, nullptr,      &toolsMenu },
  { "About", MENU_ACTION,  openAboutApp, nullptr    }
};

Menu toolsMenu = {
  "Tools",
  toolsItems,
  sizeof(toolsItems) / sizeof(toolsItems[0]),
  &mainMenu
};

Menu mainMenu = {
  "Main Menu",
  mainItems,
  sizeof(mainItems) / sizeof(mainItems[0]),
  nullptr
};

MenuApp mainMenuApp(&mainMenu, &displayManager);

// =========================
// Actions
// =========================
void goBackApp() {
  appManager.goBack();
}

void openAboutApp() {
  appManager.openApp(&aboutApp);
}

void openScanApp() {
  appManager.openApp(&scanApp);
}

void openBatteryApp() {
  appManager.openApp(&batteryApp);
}

void openSavedApp() {
  appManager.openApp(&savedApp);
}

void openKeyboardForNetwork(const char* ssid) {
  keyboardApp.setTarget(ssid, openConnectApp);
  appManager.openApp(&keyboardApp);
}

void connectToNetwork(const char* ssid, const char* password) {
  connectApp.setCredentials(ssid, password);
  appManager.openApp(&connectApp);
}

void openConnectApp(const char* ssid, const char* password) {
  connectApp.setCredentials(ssid, password);
  appManager.openApp(&connectApp);
}

void openSpeedtestApp() {
  appManager.openApp(&speedtestApp);
}

// =========================
// Setup / Loop
// =========================
void setup() {
  Serial.begin(115200);

  pinMode(8, OUTPUT);
  digitalWrite(8, HIGH);

  if (!displayManager.begin()) {
    while (true) {}
  }

  // Splash screen
  Adafruit_SSD1306& display = displayManager.getDisplay();
  display.clearDisplay();
  display.setTextColor(WHITE);

  display.setTextSize(2);
  display.setCursor(10, 8);
  display.print("WiFi");
  display.setCursor(10, 26);
  display.print("Scanner");

  display.setTextSize(1);
  display.drawLine(0, 44, 127, 44, WHITE);
  display.setCursor(0, 48);
  display.print("Antoni & Radostin");
  display.setCursor(100, 57);
  display.print("v1.0");

  display.display();
  delay(2000);

  inputManager.begin();
  appManager.setDisplayManager(&displayManager);
  appManager.openApp(&mainMenuApp);
}

void loop() {
  inputManager.update();

  InputEvent event = inputManager.getEvent();
  if (event != EVENT_NONE) {
    appManager.handleInput(event);
  }

  appManager.update();
  appManager.render();
}
