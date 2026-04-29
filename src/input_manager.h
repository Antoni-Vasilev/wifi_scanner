#pragma once

#include <OneButton.h>
#include "input_events.h"

class InputManager {
  private:
  OneButton buttonUp;
  OneButton buttonDown;
  OneButton buttonLeft;
  OneButton buttonRight;

  volatile InputEvent pendingEvent;

  static InputManager* instance;

  static void onUpClick();
  static void onDownClick();
  static void onLeftClick();
  static void onRightClick();
  static void onRightLongPress();

  void pushEvent(InputEvent event);

  public:
  InputManager();

  void begin();
  void update();

  InputEvent getEvent();
};
