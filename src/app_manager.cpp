#include "app_manager.h"

AppManager::AppManager()
  : currentApp(nullptr),
    stackTop(-1),
    displayManager(nullptr) {}

void AppManager::setDisplayManager(DisplayManager* dm) {
  displayManager = dm;
}

void AppManager::openApp(App* app) {
  if (app == nullptr) return;

  if (currentApp != nullptr && stackTop < MAX_STACK - 1) {
    currentApp->onExit();
    appStack[++stackTop] = currentApp;
  }

  currentApp = app;
  currentApp->onEnter();
}

void AppManager::goBack() {
  if (stackTop < 0) return;

  if (currentApp != nullptr) {
    currentApp->onExit();
  }

  currentApp = appStack[stackTop--];
  currentApp->onEnter();
}

void AppManager::handleInput(InputEvent event) {
  if (currentApp != nullptr) {
    currentApp->handleInput(event);
  }
}

void AppManager::update() {
  if (currentApp != nullptr) {
    currentApp->update();
  }

  if (displayManager != nullptr) {
    displayManager->updateBattery();
  }

  static unsigned long lastForceRedraw = 0;
  unsigned long now = millis();
  if (now - lastForceRedraw >= 1000) {
    lastForceRedraw = now;
    if (currentApp != nullptr) {
      currentApp->forceRedraw();
    }
  }
}

void AppManager::render() {
  if (currentApp != nullptr && currentApp->needsRedraw()) {
    currentApp->render();         // Апът рисува в буфера — без display.display()
    currentApp->clearRedrawFlag();

    if (displayManager != nullptr) {
      displayManager->drawOverlay(); // Overlay в буфера — без display.display()
    }

    // Един единствен display.display() за целия frame
    if (displayManager != nullptr) {
      displayManager->getDisplay().display();
    }
  }
}

App* AppManager::getCurrentApp() const {
  return currentApp;
}