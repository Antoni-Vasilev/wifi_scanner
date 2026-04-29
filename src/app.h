#pragma once

#include "input_events.h"

class App {
  public:
  virtual ~App() {}

  virtual void onEnter() {}
  virtual void onExit() {}
  virtual void handleInput(InputEvent event) = 0;
  virtual void update() {}
  virtual void render() = 0;

  virtual bool needsRedraw() const = 0;
  virtual void clearRedrawFlag() = 0;
  virtual void forceRedraw() {}
};
