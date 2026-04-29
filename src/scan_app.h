#pragma once

#include "app.h"
#include "display_manager.h"
#include "nvs_manager.h"
#include <WiFi.h>

#define MAX_NETWORKS 20
#define AUTO_SCAN_INTERVAL_MS 30000
#define RSSI_HISTORY_SIZE 30

struct WifiNetwork {
    char ssid[33];
    int32_t rssi;
    uint8_t channel;
    bool isOpen;
    bool isSaved;
    char bssid[18];
};

enum ScanState {
    SCAN_IDLE,
    SCAN_IN_PROGRESS,
    SCAN_DONE
};

enum ScanView {
    VIEW_LIST,
    VIEW_DETAIL
};

class ScanApp : public App {
    private:
    DisplayManager* displayManager;
    NVSManager*     nvsManager;
    bool redraw;

    WifiNetwork networks[MAX_NETWORKS];
    int networkCount;

    ScanState scanState;
    ScanView currentView;

    int selectedIndex;
    int scrollOffset;

    unsigned long lastScanTime;
    unsigned long lastCountdownSecond;

    // RSSI история за графиката
    int32_t rssiHistory[RSSI_HISTORY_SIZE];
    int rssiHistoryCount;
    unsigned long lastRssiSample;
    int lastDetailIndex;
    bool rssiScanPending;

    static const int VISIBLE_ITEMS = 5;

    void startScan();
    void checkScanResult();
    void drawRssiGraph(int x, int y, int w, int h);

    void renderList();
    void renderScanning();
    void renderDetail();

    void updateScroll();
    void moveUp();
    void moveDown();

    public:
    ScanApp(DisplayManager* dm, NVSManager* nvs);

    void onEnter() override;
    void onExit() override;
    void handleInput(InputEvent event) override;
    void update() override;
    void render() override;
    void forceRedraw() override { redraw = true; }

    bool needsRedraw() const override;
    void clearRedrawFlag() override;

    const char* getSelectedSSID() const;
    bool selectedIsOpen() const;
};
