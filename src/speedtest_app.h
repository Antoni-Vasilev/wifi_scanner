#pragma once

#include "app.h"
#include "display_manager.h"
#include <WiFi.h>
#include <lwip/sockets.h>

enum SpeedtestState {
  ST_IDLE,
  ST_SENDING,
  ST_WAITING,
  ST_DONE,
  ST_ERROR
};

struct PingResult {
  const char* host;
  int pingMs;  // -1 = timeout, -2 = pending
};

class SpeedtestApp : public App {
  private:
  DisplayManager* displayManager;
  bool redraw;

  SpeedtestState state;
  int currentHost;
  unsigned long pingStartTime;
  unsigned long dotTimer;
  int dotCount;

  int sock;  // raw socket за async ping

  static const int HOST_COUNT = 3;
  PingResult results[HOST_COUNT];

  static const char* HOSTS[];
  static const IPAddress HOST_IPS[];
  static const unsigned long PING_TIMEOUT_MS = 2000;

  bool sendPing(IPAddress ip);
  bool receivePing();
  void closeSocket();
  void startNextPing();

  public:
  SpeedtestApp(DisplayManager* dm);

  void onEnter() override;
  void handleInput(InputEvent event) override;
  void update() override;
  void render() override;
  void forceRedraw() override { redraw = true; }

  bool needsRedraw() const override;
  void clearRedrawFlag() override;
};
