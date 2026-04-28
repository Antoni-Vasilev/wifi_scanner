#pragma once

#include "app.h"
#include "display_manager.h"

class BatteryApp : public App {
  private:
  DisplayManager* displayManager;
  bool redraw;

  int lastRenderedPercent;
  bool lastRenderedCharging;

  public:
  BatteryApp(DisplayManager* dm);

  void onEnter() override;
  void handleInput(InputEvent event) override;
  void render() override;
  void forceRedraw() override;

  bool needsRedraw() const override;
  void clearRedrawFlag() override;
};