#include <Arduino.h>

#include "config.h"
#include "src/app_manager.h"
#include "src/input_manager.h"
#include "src/display_manager.h"
#include "src/menu_app.h"
#include "src/menu_types.h"
#include "src/about_app.h"

// =========================
// Global managers
// =========================
DisplayManager displayManager;
InputManager inputManager;
AppManager appManager;

// =========================
// App instances
// =========================
AboutApp aboutApp(&displayManager);
MenuApp* menuApp = nullptr;

// =========================
// Navigation helpers
// =========================
void openAboutApp();
void goBackApp();

// =========================
// Menu definitions
// =========================
extern Menu toolsMenu;
extern Menu mainMenu;

MenuItem toolsItems[] = {
  { "Back", MENU_BACK, nullptr, nullptr }
};

MenuItem mainItems[] = {
  { "Tools", MENU_SUBMENU, nullptr, &toolsMenu },
  { "About", MENU_ACTION, openAboutApp, nullptr }
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

// =========================
// Menu app instance
// =========================
MenuApp mainMenuApp(&mainMenu, &displayManager);

// =========================
// Actions
// =========================
void openAboutApp() {
  appManager.openApp(&aboutApp);
}

void goBackApp() {
  appManager.goBack();
}

// =========================
// Setup / loop
// =========================

void setup() {
  Serial.begin(115200);

  if (!displayManager.begin()) {
    while (true) {
    }
  }

  inputManager.begin();

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