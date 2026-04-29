#include "connect_app.h"
#include <Adafruit_SSD1306.h>
#include <esp_wifi.h>

extern void goBackApp();
extern void openSpeedtestApp();

ConnectApp::ConnectApp(DisplayManager* dm, NVSManager* nvs)
  : displayManager(dm),
    nvsManager(nvs),
    redraw(true),
    state(CONNECT_CONNECTING),
    connectStartTime(0),
    dotTimer(0),
    dotCount(0),
    networkSaved(false) {
  ssid[0]     = '\0';
  password[0] = '\0';
}

void ConnectApp::setCredentials(const char* targetSsid, const char* targetPassword) {
  strncpy(ssid, targetSsid, 32);
  ssid[32] = '\0';
  strncpy(password, targetPassword, 64);
  password[64] = '\0';
}

void ConnectApp::onEnter() {
  // Ако вече сме свързани към същата мрежа — не се свързвай отново
  if (WiFi.status() == WL_CONNECTED &&
      String(WiFi.SSID()) == String(ssid)) {
    state  = CONNECT_SUCCESS;
    redraw = true;
    return;
  }

  state            = CONNECT_CONNECTING;
  connectStartTime = millis();
  dotCount         = 0;
  networkSaved     = false;
  redraw           = true;

  WiFi.disconnect(true);
  delay(100);
  WiFi.mode(WIFI_STA);
  esp_wifi_set_max_tx_power(40);

  if (strlen(password) > 0) {
    WiFi.begin(ssid, password);
  } else {
    WiFi.begin(ssid);
  }
}

void ConnectApp::onExit() {
  // Не прекъсваме връзката при излизане
}

void ConnectApp::handleInput(InputEvent event) {
  if (state == CONNECT_SUCCESS) {
    if (event == EVENT_RIGHT) {
      openSpeedtestApp();
    } else if (event == EVENT_LEFT) {
      goBackApp();
    }
  } else if (state == CONNECT_FAILED) {
    if (event == EVENT_LEFT) {
      goBackApp();
    }
  }
}

void ConnectApp::update() {
  if (state != CONNECT_CONNECTING) return;

  unsigned long now = millis();

  if (now - dotTimer >= 400) {
    dotTimer = now;
    dotCount = (dotCount + 1) % 4;
    redraw   = true;
  }

  wl_status_t status = WiFi.status();

  if (status == WL_CONNECTED) {
    state = CONNECT_SUCCESS;

    // Запази мрежата в NVS само при успешна връзка
    networkSaved = nvsManager->saveNetwork(ssid, password);

    redraw = true;
    return;
  }

  if (status == WL_CONNECT_FAILED ||
      status == WL_NO_SSID_AVAIL  ||
      now - connectStartTime >= CONNECT_TIMEOUT_MS) {
    state  = CONNECT_FAILED;
    redraw = true;
    return;
  }
}

void ConnectApp::render() {
  Adafruit_SSD1306& display = displayManager->getDisplay();

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  // Header
  char header[19];
  strncpy(header, ssid, 18);
  header[18] = '\0';
  display.setCursor(0, 0);
  display.print(header);
  display.drawLine(0, 10, 127, 10, WHITE);

  if (state == CONNECT_CONNECTING) {
    display.setCursor(0, 20);
    display.print("Connecting");
    for (int i = 0; i < dotCount; i++) display.print(".");

    display.setCursor(0, 32);
    display.print("SSID: ");
    display.println(ssid);
  }
  else if (state == CONNECT_SUCCESS) {
    display.setCursor(0, 13);
    display.println("Connected!");

    display.setCursor(0, 25);
    display.print("IP: ");
    display.println(WiFi.localIP());

    // Индикация дали е запазена
    display.setCursor(0, 37);
    if (networkSaved) {
      display.print("Saved to memory");
    }

    // Ping test бутон
    display.drawRect(0, 46, 128, 11, WHITE);
    display.setCursor(20, 48);
    display.print("RIGHT = Ping Test");

    display.setCursor(0, 57);
    display.print("LEFT = Back");
  }
  else if (state == CONNECT_FAILED) {
    display.setCursor(0, 18);
    display.println("Failed!");

    display.setCursor(0, 30);
    display.println("Check password");
    display.println("and try again.");

    display.setCursor(0, 57);
    display.print("LEFT = Back");
  }

  // БЕЗ display.display() — извиква се от AppManager
}

bool ConnectApp::needsRedraw() const { return redraw; }
void ConnectApp::clearRedrawFlag() { redraw = false; }
