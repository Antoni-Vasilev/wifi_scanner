#include "scan_app.h"
#include <Adafruit_SSD1306.h>
#include <esp_wifi.h>

extern void goBackApp();
extern void openKeyboardForNetwork(const char* ssid);
extern void connectToNetwork(const char* ssid, const char* password);

ScanApp::ScanApp(DisplayManager* dm, NVSManager* nvs)
    : displayManager(dm),
      nvsManager(nvs),
      redraw(true),
      networkCount(0),
      scanState(SCAN_IDLE),
      currentView(VIEW_LIST),
      selectedIndex(0),
      scrollOffset(0),
      lastScanTime(0),
      lastCountdownSecond(0),
      rssiHistoryCount(0),
      lastRssiSample(0),
      lastDetailIndex(-1),
      rssiScanPending(false) {
    memset(rssiHistory, 0, sizeof(rssiHistory));
}

void ScanApp::onEnter() {
    currentView      = VIEW_LIST;
    selectedIndex    = 0;
    scrollOffset     = 0;
    rssiHistoryCount = 0;
    lastDetailIndex  = -1;
    rssiScanPending  = false;
    redraw           = true;
    startScan();
}

void ScanApp::onExit() {
    if (scanState == SCAN_IN_PROGRESS || rssiScanPending) {
        WiFi.scanDelete();
        scanState       = SCAN_IDLE;
        rssiScanPending = false;
    }
}

void ScanApp::handleInput(InputEvent event) {
    if (scanState == SCAN_IN_PROGRESS) return;

    if (currentView == VIEW_LIST) {
        switch (event) {
            case EVENT_UP:   moveUp();   break;
            case EVENT_DOWN: moveDown(); break;
            case EVENT_RIGHT: {
                bool onRescan = (selectedIndex == networkCount);
                if (onRescan) {
                    startScan();
                } else if (networkCount > 0) {
                    if (selectedIndex != lastDetailIndex) {
                        rssiHistoryCount = 0;
                        lastDetailIndex  = selectedIndex;
                        rssiScanPending  = false;
                    }
                    currentView = VIEW_DETAIL;
                    redraw      = true;
                }
                break;
            }
            case EVENT_LEFT:
                goBackApp();
                break;
            default: break;
        }
    } else if (currentView == VIEW_DETAIL) {
        switch (event) {
            case EVENT_LEFT:
                if (rssiScanPending) {
                    WiFi.scanDelete();
                    rssiScanPending = false;
                }
                currentView = VIEW_LIST;
                redraw      = true;
                break;

            case EVENT_UP:
                if (networks[selectedIndex].isSaved) {
                    nvsManager->deleteNetwork(networks[selectedIndex].ssid);
                    networks[selectedIndex].isSaved = false;
                    redraw = true;
                }
                break;

            case EVENT_RIGHT: {
                WifiNetwork& net = networks[selectedIndex];
                if (net.isSaved) {
                    SavedNetwork saved;
                    if (nvsManager->findNetwork(net.ssid, saved)) {
                        connectToNetwork(saved.ssid, saved.password);
                    }
                } else if (net.isOpen) {
                    connectToNetwork(net.ssid, "");
                } else {
                    openKeyboardForNetwork(net.ssid);
                }
                break;
            }
            default: break;
        }
    }
}

void ScanApp::update() {
    if (scanState == SCAN_IN_PROGRESS) {
        checkScanResult();
        return;
    }

    if (scanState == SCAN_DONE) {
        unsigned long now = millis();

        if (currentView == VIEW_LIST && now - lastScanTime >= AUTO_SCAN_INTERVAL_MS) {
            startScan();
            return;
        }

        unsigned long currentSecond = (AUTO_SCAN_INTERVAL_MS - (now - lastScanTime)) / 1000;
        if (currentSecond != lastCountdownSecond) {
            lastCountdownSecond = currentSecond;
            redraw = true;
        }

        if (currentView == VIEW_DETAIL && selectedIndex < networkCount) {
            bool isConnectedToThis = (WiFi.status() == WL_CONNECTED &&
                String(WiFi.SSID()) == String(networks[selectedIndex].ssid));

            if (isConnectedToThis) {
                if (now - lastRssiSample >= 150) {
                    lastRssiSample = now;

                    wifi_ap_record_t ap_info;
                    int32_t newRssi = networks[selectedIndex].rssi;

                    if (esp_wifi_sta_get_ap_info(&ap_info) == ESP_OK) {
                        newRssi = ap_info.rssi;
                    }

                    networks[selectedIndex].rssi = newRssi;

                    if (rssiHistoryCount >= RSSI_HISTORY_SIZE) {
                        for (int i = 0; i < RSSI_HISTORY_SIZE - 1; i++) {
                            rssiHistory[i] = rssiHistory[i + 1];
                        }
                        rssiHistory[RSSI_HISTORY_SIZE - 1] = newRssi;
                    } else {
                        rssiHistory[rssiHistoryCount++] = newRssi;
                    }
                    redraw = true;
                }
            } else {
                if (!rssiScanPending && now - lastRssiSample >= 2000) {
                    lastRssiSample  = now;
                    WiFi.scanNetworks(true, true);
                    rssiScanPending = true;
                }

                if (rssiScanPending) {
                    int result = WiFi.scanComplete();
                    if (result != WIFI_SCAN_RUNNING && result >= 0) {
                        int32_t newRssi = networks[selectedIndex].rssi;
                        for (int i = 0; i < result; i++) {
                            if (WiFi.SSID(i) == String(networks[selectedIndex].ssid)) {
                                newRssi = WiFi.RSSI(i);
                                break;
                            }
                        }
                        WiFi.scanDelete();
                        rssiScanPending              = false;
                        networks[selectedIndex].rssi = newRssi;

                        if (rssiHistoryCount >= RSSI_HISTORY_SIZE) {
                            for (int i = 0; i < RSSI_HISTORY_SIZE - 1; i++) {
                                rssiHistory[i] = rssiHistory[i + 1];
                            }
                            rssiHistory[RSSI_HISTORY_SIZE - 1] = newRssi;
                        } else {
                            rssiHistory[rssiHistoryCount++] = newRssi;
                        }
                        redraw = true;
                    }
                }
            }
        }
    }
}

void ScanApp::startScan() {
    WiFi.mode(WIFI_STA);
    esp_wifi_set_max_tx_power(40);
    WiFi.disconnect();
    WiFi.scanNetworks(true);

    scanState        = SCAN_IN_PROGRESS;
    networkCount     = 0;
    selectedIndex    = 0;
    scrollOffset     = 0;
    rssiScanPending  = false;
    redraw           = true;
}

void ScanApp::checkScanResult() {
    int result = WiFi.scanComplete();

    if (result == WIFI_SCAN_RUNNING) return;

    if (result == WIFI_SCAN_FAILED || result < 0) {
        networkCount = 0;
        scanState    = SCAN_DONE;
        lastScanTime = millis();
        redraw       = true;
        return;
    }

    networkCount = result > MAX_NETWORKS ? MAX_NETWORKS : result;

    for (int i = 0; i < networkCount; i++) {
        strncpy(networks[i].ssid, WiFi.SSID(i).c_str(), 32);
        networks[i].ssid[32] = '\0';
        networks[i].rssi     = WiFi.RSSI(i);
        networks[i].channel  = WiFi.channel(i);
        networks[i].isOpen   = (WiFi.encryptionType(i) == WIFI_AUTH_OPEN);

        SavedNetwork tmp;
        networks[i].isSaved = nvsManager->findNetwork(networks[i].ssid, tmp);

        strncpy(networks[i].bssid, WiFi.BSSIDstr(i).c_str(), 17);
        networks[i].bssid[17] = '\0';
    }

    WiFi.scanDelete();

    scanState           = SCAN_DONE;
    lastScanTime        = millis();
    lastCountdownSecond = AUTO_SCAN_INTERVAL_MS / 1000;
    redraw              = true;
}

void ScanApp::drawRssiGraph(int x, int y, int w, int h) {
    Adafruit_SSD1306& display = displayManager->getDisplay();

    if (rssiHistoryCount < 2) {
        display.setCursor(x, y + h / 2 - 3);
        display.print("Collecting...");
        return;
    }

    int32_t minRssi = rssiHistory[0];
    int32_t maxRssi = rssiHistory[0];
    for (int i = 1; i < rssiHistoryCount; i++) {
        if (rssiHistory[i] < minRssi) minRssi = rssiHistory[i];
        if (rssiHistory[i] > maxRssi) maxRssi = rssiHistory[i];
    }

    if (maxRssi - minRssi < 10) {
        minRssi = maxRssi - 10;
    }

    int count    = min(rssiHistoryCount, RSSI_HISTORY_SIZE);
    int barWidth = w / RSSI_HISTORY_SIZE;
    if (barWidth < 1) barWidth = 1;

    for (int i = 0; i < count; i++) {
        int32_t val = rssiHistory[i];
        int barH    = map(val, minRssi, maxRssi, 1, h);
        if (barH < 1) barH = 1;
        if (barH > h) barH = h;

        int bx = x + i * barWidth;
        int by = y + h - barH;
        display.fillRect(bx, by, barWidth > 1 ? barWidth - 1 : 1, barH, WHITE);
    }
}

void ScanApp::render() {
    if (scanState == SCAN_IN_PROGRESS) {
        renderScanning();
    } else if (currentView == VIEW_DETAIL && networkCount > 0) {
        renderDetail();
    } else {
        renderList();
    }
}

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
}

void ScanApp::renderList() {
    Adafruit_SSD1306& display = displayManager->getDisplay();

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);

    display.setCursor(0, 0);
    display.print("WiFi Scanner");
    display.drawLine(0, 10, 127, 10, WHITE);

    if (scanState == SCAN_DONE) {
        unsigned long elapsed   = millis() - lastScanTime;
        unsigned long remaining = elapsed < AUTO_SCAN_INTERVAL_MS
            ? (AUTO_SCAN_INTERVAL_MS - elapsed) / 1000 : 0;
        char countdownStr[8];
        snprintf(countdownStr, sizeof(countdownStr), "%lus", remaining);
        int countdownX = 112 - strlen(countdownStr) * 6;
        display.setCursor(countdownX, 0);
        display.print(countdownStr);
    }

    int total            = networkCount + 1;
    const int itemHeight = 10;
    const int startY     = 13;
    const int SCROLLBAR_X   = 121;
    const int CONTENT_WIDTH = 120;

    int endIndex = scrollOffset + VISIBLE_ITEMS;
    if (endIndex > total) endIndex = total;

    for (int i = scrollOffset; i < endIndex; i++) {
        int visibleIndex = i - scrollOffset;
        int y = startY + visibleIndex * itemHeight;

        bool isRescan   = (i == networkCount);
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
            int iconWidth = (networks[i].isSaved || !networks[i].isOpen) ? 6 : 0;

            int ssidMaxPx    = CONTENT_WIDTH - rssiWidth - iconWidth - 8;
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

            int rssiX = CONTENT_WIDTH - rssiWidth - iconWidth - 2;
            display.setCursor(rssiX, y);
            display.print(rssiStr);

            if (networks[i].isSaved) {
                display.setCursor(CONTENT_WIDTH - 6, y);
                display.print("S");
            } else if (!networks[i].isOpen) {
                display.setCursor(CONTENT_WIDTH - 6, y);
                display.print("*");
            }
        }
    }

    if (total > VISIBLE_ITEMS) {
        int barHeight = (VISIBLE_ITEMS * 40) / total;
        if (barHeight < 4) barHeight = 4;
        int barY = 14 + (scrollOffset * 40) / total;
        display.setTextColor(WHITE);
        display.fillRect(SCROLLBAR_X, barY, 3, barHeight, WHITE);
    }
}

void ScanApp::renderDetail() {
    Adafruit_SSD1306& display = displayManager->getDisplay();

    WifiNetwork& net = networks[selectedIndex];

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);

    // Header
    char header[17];
    strncpy(header, net.ssid, 16);
    header[16] = '\0';
    display.setCursor(0, 0);
    display.print(header);
    display.drawLine(0, 10, 127, 10, WHITE);

    // RSSI + Channel
    display.setCursor(0, 13);
    display.print("RSSI:");
    display.print(net.rssi);
    display.print(" Ch:");
    display.println(net.channel);

    // Encryption
    display.print("Enc:  ");
    display.println(net.isOpen ? "Open" : "WPA/WPA2");

    // MAC съкратен
    char macShort[14];
    strncpy(macShort, net.bssid, 11);
    macShort[11] = '.';
    macShort[12] = '.';
    macShort[13] = '\0';
    display.print("MAC:  ");
    display.println(macShort);

    // Save статус
    display.print("Save: ");
    if (net.isSaved) {
        display.print("Yes  UP=del");
    } else {
        display.print("No");
    }

    // Hint
    display.setCursor(0, 44);
    if (net.isSaved) {
        display.print("R=Connect  L=Back");
    } else if (net.isOpen) {
        display.print("R=Connect  L=Back");
    } else {
        display.print("R=Password  L=Back");
    }

    // Графика
    display.drawLine(0, 53, 127, 53, WHITE);
    drawRssiGraph(0, 55, 118, 8);

    // Текущо RSSI до графиката
    display.setCursor(102, 56);
    display.print(net.rssi);
}

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

const char* ScanApp::getSelectedSSID() const {
    if (selectedIndex >= 0 && selectedIndex < networkCount) {
        return networks[selectedIndex].ssid;
    }
    return "";
}

bool ScanApp::selectedIsOpen() const {
    if (selectedIndex >= 0 && selectedIndex < networkCount) {
        return networks[selectedIndex].isOpen;
    }
    return false;
}

bool ScanApp::needsRedraw() const { return redraw; }

void ScanApp::clearRedrawFlag() {
    if (scanState == SCAN_IN_PROGRESS) return;

    if (currentView == VIEW_DETAIL && selectedIndex < networkCount) {
        bool isConnectedToThis = (WiFi.status() == WL_CONNECTED &&
            String(WiFi.SSID()) == String(networks[selectedIndex].ssid));
        if (isConnectedToThis) return;
    }

    redraw = false;
}