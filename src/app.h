#pragma once

enum InputEvent {
  EVENT_NONE,
  EVENT_UP,
  EVENT_DOWN,
  EVENT_LEFT,
  EVENT_RIGHT
};

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