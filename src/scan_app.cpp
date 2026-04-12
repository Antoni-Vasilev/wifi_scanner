#include "scan_app.h"
#include <Adafruit_SSD1306.h>

extern void goBackApp();

ScanApp::ScanApp(DisplayManager* dm)
    : displayManager(dm), 
      redraw(true),
      networkCount(0),
      scanState(SCAN_IDLE),
      currentView(VIEW_LIST),
      selectedIndex(0),
      scrollOffset(0),
      lastScanTime(0),
      lastCountdownSecond(0) {}

// =========================
// Lifecycle
// =========================

void ScanApp::onEnter() {
    currentView = VIEW_LIST;
    selectedIndex = 0;
    scrollOffset = 0;
    redraw = true;
    startScan();
}

void ScanApp::onExit() {
    if (scanState == SCAN_IN_PROGRESS) {
        WiFi.scanDelete();
        scanState = SCAN_IDLE;
    }
}

// =========================
// Input
// =========================

void ScanApp::handleInput(InputEvent event) {
    if (scanState == SCAN_IN_PROGRESS) return;

    if (currentView == VIEW_LIST) {
        switch (event) {
            case EVENT_UP:
                moveUp();
                break;
            case EVENT_DOWN:
                moveDown();
                break;
            case EVENT_RIGHT: {
                bool onRescan = (selectedIndex == networkCount);
                if (onRescan) {
                    startScan();
                } else if (networkCount > 0) {
                    currentView = VIEW_DETAIL;
                    redraw = true;
                }
                break;
            }
            case EVENT_LEFT:
                goBackApp();
                break;
            default:
                break;
        }
    } else if (currentView == VIEW_DETAIL) {
        if (event == EVENT_LEFT || event == EVENT_RIGHT) {
            currentView = VIEW_LIST;
            redraw = true;
        }
    }
}

// =========================
// Update (called every loop)
// =========================

void ScanApp::update() {
    if (scanState == SCAN_IN_PROGRESS) {
        checkScanResult();
        return;
    }

    if (scanState == SCAN_DONE) {
        unsigned long now = millis();

        // Auto-rescan
        if (currentView == VIEW_LIST && now - lastScanTime >= AUTO_SCAN_INTERVAL_MS) {
            startScan();
            return;
        }

        // Обнови само когато секундата се смени
        unsigned long currentSecond = (AUTO_SCAN_INTERVAL_MS - (now - lastScanTime)) / 1000;
        if (currentSecond != lastCountdownSecond) {
            lastCountdownSecond = currentSecond;
            redraw = true;
        }
    }
}

// =========================
// Scan logic
// =========================

void ScanApp::startScan() {
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    WiFi.scanNetworks(true);

    scanState = SCAN_IN_PROGRESS;
    networkCount = 0;
    selectedIndex = 0;
    scrollOffset = 0;
    redraw = true;
}

void ScanApp::checkScanResult() {
    int result = WiFi.scanComplete();

    if (result == WIFI_SCAN_RUNNING) return;

    if (result == WIFI_SCAN_FAILED || result < 0) {
        networkCount = 0;
        scanState = SCAN_DONE;
        lastScanTime = millis();
        redraw = true;
        return;
    }

    networkCount = result > MAX_NETWORKS ? MAX_NETWORKS : result;

    for (int i = 0; i < networkCount; i++) {
        strncpy(networks[i].ssid, WiFi.SSID(i).c_str(), 32);
        networks[i].ssid[32] = '\0';

        networks[i].rssi = WiFi.RSSI(i);
        networks[i].channel = WiFi.channel(i);
        networks[i].isOpen = (WiFi.encryptionType(i) == WIFI_AUTH_OPEN);

        strncpy(networks[i].bssid, WiFi.BSSIDstr(i).c_str(), 17);
        networks[i].bssid[17] = '\0';
    }

    WiFi.scanDelete();

    scanState = SCAN_DONE;
    lastScanTime = millis();
    lastCountdownSecond = AUTO_SCAN_INTERVAL_MS / 1000;
    redraw = true;
}

// =========================
// Render dispatch
// =========================

void ScanApp::render() {
    if (scanState == SCAN_IN_PROGRESS) {
        renderScanning();
    } else if (currentView == VIEW_DETAIL && networkCount > 0) {
        renderDetail();
    } else {
        renderList();
    }
}

// =========================
// Render: Scanning screen
// =========================

void ScanApp::renderScanning() {
    Adafruit_SSD1306& display = displayManager->getDisplay();

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);

    display.setCursor(0, 0);
    display.print("WiFi Scanner");
    display.drawLine(0, 10, 127, 10, WHITE);

    display.setCursor(0, 20);
    display.println("Scanning...");

    int dots = (millis() / 400) % 4;
    display.setCursor(0, 32);
    for (int i = 0; i < dots; i++) display.print(".");

    display.display();

    redraw = true;
}

// =========================
// Render: List view
// =========================

void ScanApp::renderList() {
    Adafruit_SSD1306& display = displayManager->getDisplay();

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);

    display.setCursor(0, 0);
    display.print("WiFi Scanner");
    display.drawLine(0, 10, 127, 10, WHITE);

    // Countdown
    if (scanState == SCAN_DONE) {
        unsigned long elapsed = millis() - lastScanTime;
        unsigned long remaining = 0;
        if (elapsed < AUTO_SCAN_INTERVAL_MS) {
            remaining = (AUTO_SCAN_INTERVAL_MS - elapsed) / 1000;
        }
        char countdownStr[8];
        snprintf(countdownStr, sizeof(countdownStr), "%lus", remaining);
        int countdownX = 128 - strlen(countdownStr) * 6;
        display.setCursor(countdownX, 0);
        display.print(countdownStr);
    }

    int total = networkCount + 1;
    const int itemHeight = 10;
    const int startY = 13;

    const int SCROLLBAR_X = 121;
    const int CONTENT_WIDTH = 120;

    int endIndex = scrollOffset + VISIBLE_ITEMS;
    if (endIndex > total) endIndex = total;

    for (int i = scrollOffset; i < endIndex; i++) {
        int visibleIndex = i - scrollOffset;
        int y = startY + visibleIndex * itemHeight;

        bool isRescan = (i == networkCount);
        bool isSelected = (i == selectedIndex);

        if (isSelected) {
            display.fillRect(0, y - 1, CONTENT_WIDTH, 9, WHITE);
            display.setTextColor(BLACK);
        } else {
            display.setTextColor(WHITE);
        }

        if (isRescan) {
            display.setCursor(3, y);
            display.print("Rescan");
            display.setCursor(CONTENT_WIDTH - 7, y);
            display.print(">");
        } else {
            char rssiStr[8];
            snprintf(rssiStr, sizeof(rssiStr), "%d", networks[i].rssi);
            int rssiWidth = strlen(rssiStr) * 6;
            int lockWidth = networks[i].isOpen ? 0 : 8;

            int ssidMaxPx = CONTENT_WIDTH - rssiWidth - lockWidth - 6;
            int ssidMaxChars = ssidMaxPx / 6;

            char label[18];
            strncpy(label, networks[i].ssid, ssidMaxChars);
            label[ssidMaxChars] = '\0';

            if (strlen(networks[i].ssid) > (size_t)ssidMaxChars && ssidMaxChars >= 3) {
                label[ssidMaxChars - 3] = '.';
                label[ssidMaxChars - 2] = '.';
                label[ssidMaxChars - 1] = '.';
            }

            display.setCursor(2, y);
            display.print(label);

            int rssiX = CONTENT_WIDTH - rssiWidth - lockWidth - 2;
            display.setCursor(rssiX, y);
            display.print(rssiStr);

            if (!networks[i].isOpen) {
                display.setCursor(CONTENT_WIDTH - 7, y);
                display.print("*");
            }
        }
    }

    // Scrollbar
    if (total > VISIBLE_ITEMS) {
        int barHeight = (VISIBLE_ITEMS * 40) / total;
        if (barHeight < 4) barHeight = 4;
        int barY = 14 + (scrollOffset * 40) / total;
        display.setTextColor(WHITE);
        display.fillRect(SCROLLBAR_X, barY, 3, barHeight, WHITE);
    }

    display.display();
}

// =========================
// Render: Detail view
// =========================

void ScanApp::renderDetail() {
    Adafruit_SSD1306& display = displayManager->getDisplay();

    WifiNetwork& net = networks[selectedIndex];

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);

    char header[17];
    strncpy(header, net.ssid, 16);
    header[16] = '\0';

    display.setCursor(0, 0);
    display.print(header);
    display.drawLine(0, 10, 127, 10, WHITE);

    display.setCursor(0, 13);

    display.print("RSSI: ");
    display.print(net.rssi);
    display.println(" dBm");

    display.print("Chan: ");
    display.println(net.channel);

    display.print("Enc: ");
    display.println(net.isOpen ? "Open" : "WPA/WPA2");

    display.print("MAC: ");
    display.println(net.bssid);

    display.setCursor(0, 57);
    display.print("LEFT/RIGHT = Back");

    display.display();
}

// =========================
// Scroll helpers
// =========================

void ScanApp::moveUp() {
    int total = networkCount + 1;
    if (total == 0) return;

    if (selectedIndex > 0) selectedIndex--;
    else selectedIndex = total - 1;

    updateScroll();
    redraw = true;
}

void ScanApp::moveDown() {
    int total = networkCount + 1;
    if (total == 0) return;

    if (selectedIndex < total - 1) selectedIndex++;
    else selectedIndex = 0;

    updateScroll();
    redraw = true;
}

void ScanApp::updateScroll() {
    int total = networkCount + 1;
    if (selectedIndex < scrollOffset) {
        scrollOffset = selectedIndex;
    } else if (selectedIndex >= scrollOffset + VISIBLE_ITEMS) {
        scrollOffset = selectedIndex - VISIBLE_ITEMS + 1;
    }

    int maxScroll = total - VISIBLE_ITEMS;
    if (maxScroll < 0) maxScroll = 0;
    if (scrollOffset > maxScroll) scrollOffset = maxScroll;
}

// =========================
// Redraw flags
// =========================

bool ScanApp::needsRedraw() const {
    return redraw;
}

void ScanApp::clearRedrawFlag() {
    if (scanState != SCAN_IN_PROGRESS) {
        redraw = false;
    }
}