#pragma once

#include "app.h"
#include "display_manager.h"

#define MAX_PASSWORD_LEN 64

typedef void (*KeyboardCallback)(const char* ssid, const char* password);

class KeyboardApp : public App {
  private:
  DisplayManager* displayManager;
  bool redraw;

  char ssid[33];
  char password[MAX_PASSWORD_LEN + 1];
  int passwordLen;
  int symbolIndex;

  KeyboardCallback onConfirm;

  public:
  static const char SYMBOLS[];
  static const int SYMBOL_COUNT_VAL = 89; // a-z(26) + A-Z(26) + 0-9(10) + special(26) + space(1)
  // IDX_DEL = SYMBOL_COUNT_VAL, IDX_OK = SYMBOL_COUNT_VAL + 1
  static const int TOTAL_ITEMS = SYMBOL_COUNT_VAL + 2;

  KeyboardApp(DisplayManager* dm);

  void setTarget(const char* ssid, KeyboardCallback callback);

  void onEnter() override;
  void handleInput(InputEvent event) override;
  void render() override;
  void forceRedraw() override { redraw = true; }

  bool needsRedraw() const override;
  void clearRedrawFlag() override;
};