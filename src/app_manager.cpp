#include "app_manager.h"

AppManager::AppManager() : currentApp(nullptr), stackTop(-1) {}

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
}

void AppManager::render() {
  if (currentApp != nullptr && currentApp->needsRedraw()) {
    currentApp->render();
    currentApp->clearRedrawFlag();
  }
}

App* AppManager::getCurrentApp() const {
  return currentApp;
}