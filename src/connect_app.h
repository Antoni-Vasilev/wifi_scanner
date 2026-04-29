#pragma once

#include "app.h"
#include "display_manager.h"
#include "nvs_manager.h"
#include <WiFi.h>

enum ConnectState {
  CONNECT_CONNECTING,
  CONNECT_SUCCESS,
  CONNECT_FAILED
};

class ConnectApp : public App {
  private:
  DisplayManager* displayManager;
  NVSManager* nvsManager;
  bool redraw;

  char ssid[33];
  char password[65];

  ConnectState state;
  unsigned long connectStartTime;
  unsigned long dotTimer;
  int dotCount;

  bool networkSaved; // дали е запазена след успех

  static const unsigned long CONNECT_TIMEOUT_MS = 10000;

  public:
  ConnectApp(DisplayManager* dm, NVSManager* nvs);

  void setCredentials(const char* ssid, const char* password);

  void onEnter() override;
  void onExit() override;
  void handleInput(InputEvent event) override;
  void update() override;
  void render() override;
  void forceRedraw() override { redraw = true; }

  bool needsRedraw() const override;
  void clearRedrawFlag() override;
};
