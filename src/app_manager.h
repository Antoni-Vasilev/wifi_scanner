#pragma once

#include "app.h"

class AppManager {
  private:
  static const int MAX_STACK = 10;

  App* currentApp;
  App* appStack[MAX_STACK];
  int stackTop;

  public:
  AppManager();

  void openApp(App* app);
  void goBack();

  void handleInput(InputEvent event);
  void update();
  void render();

  App* getCurrentApp() const;
};