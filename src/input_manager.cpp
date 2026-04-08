#include "input_manager.h"
#include "../config.h"

InputManager* InputManager::instance = nullptr;

InputManager::InputManager()
  : buttonUp(BTN_UP, true),
    buttonDown(BTN_DOWN, true),
    buttonLeft(BTN_LEFT, true),
    buttonRight(BTN_RIGHT, true),
    pendingEvent(EVENT_NONE) 
  {
      instance = this;
}

void InputManager::begin() {
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_LEFT, INPUT_PULLUP);
  pinMode(BTN_RIGHT, INPUT_PULLUP);

  buttonUp.setDebounceMs(25);
  buttonDown.setDebounceMs(25);
  buttonLeft.setDebounceMs(25);
  buttonRight.setDebounceMs(25);

  buttonUp.attachClick(onUpClick);
  buttonDown.attachClick(onDownClick);
  buttonLeft.attachClick(onLeftClick);
  buttonRight.attachClick(onRightClick);
}

void InputManager::update() {
  buttonUp.tick();
  buttonDown.tick();
  buttonLeft.tick();
  buttonRight.tick();
}

InputEvent InputManager::getEvent() {
  InputEvent event = pendingEvent;
  pendingEvent = EVENT_NONE;
  return event;
}

void InputManager::pushEvent(InputEvent event) {
  pendingEvent = event;
}

void InputManager::onUpClick() {
  if (instance) instance->pushEvent(EVENT_UP);
}

void InputManager::onDownClick() {
  if (instance) instance->pushEvent(EVENT_DOWN);
}

void InputManager::onLeftClick() {
  if (instance) instance->pushEvent(EVENT_LEFT);
}

void InputManager::onRightClick() {
  if (instance) instance->pushEvent(EVENT_RIGHT);
}