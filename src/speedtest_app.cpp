#include "speedtest_app.h"
#include <Adafruit_SSD1306.h>
#include <lwip/sockets.h>
#include <lwip/netdb.h>

extern void goBackApp();

const char* SpeedtestApp::HOSTS[] = {
  "8.8.8.8",
  "1.1.1.1",
  "8.8.4.4"
};

const IPAddress SpeedtestApp::HOST_IPS[] = {
  IPAddress(8,  8,  8,  8),
  IPAddress(1,  1,  1,  1),
  IPAddress(8,  8,  4,  4)
};

SpeedtestApp::SpeedtestApp(DisplayManager* dm)
  : displayManager(dm),
    redraw(true),
    state(ST_IDLE),
    currentHost(0),
    pingStartTime(0),
    dotTimer(0),
    dotCount(0),
    sock(-1) {
  for (int i = 0; i < HOST_COUNT; i++) {
    results[i].host   = HOSTS[i];
    results[i].pingMs = -2; // pending
  }
}

void SpeedtestApp::onEnter() {
  closeSocket();
  currentHost = 0;
  dotCount    = 0;
  state       = ST_IDLE;

  for (int i = 0; i < HOST_COUNT; i++) {
    results[i].pingMs = -2;
  }

  redraw = true;
  startNextPing();
}

void SpeedtestApp::closeSocket() {
  if (sock >= 0) {
    close(sock);
    sock = -1;
  }
}

void SpeedtestApp::startNextPing() {
  if (currentHost >= HOST_COUNT) {
    state  = ST_DONE;
    redraw = true;
    return;
  }

  // Отвори non-blocking raw socket
  sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
  if (sock < 0) {
    results[currentHost].pingMs = -1;
    currentHost++;
    startNextPing();
    return;
  }

  // Non-blocking
  int flags = fcntl(sock, F_GETFL, 0);
  fcntl(sock, F_SETFL, flags | O_NONBLOCK);

  // Изгради ICMP пакет
  uint8_t packet[64];
  memset(packet, 0, sizeof(packet));
  packet[0] = 8;  // echo request
  packet[1] = 0;
  packet[4] = 0;
  packet[5] = (uint8_t)currentHost + 1;
  packet[6] = 0;
  packet[7] = 1;

  // Checksum
  uint32_t sum = 0;
  for (int i = 0; i < 64; i += 2) {
    sum += ((uint16_t)packet[i] << 8) | packet[i + 1];
  }
  while (sum >> 16) sum = (sum & 0xFFFF) + (sum >> 16);
  sum = ~sum;
  packet[2] = (sum >> 8) & 0xFF;
  packet[3] = sum & 0xFF;

  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family      = AF_INET;
  addr.sin_addr.s_addr = (uint32_t)HOST_IPS[currentHost];

  pingStartTime = millis();

  int sent = sendto(sock, packet, sizeof(packet), 0,
                    (struct sockaddr*)&addr, sizeof(addr));

  if (sent < 0) {
    closeSocket();
    results[currentHost].pingMs = -1;
    currentHost++;
    startNextPing();
    return;
  }

  state  = ST_WAITING;
  redraw = true;
}

bool SpeedtestApp::receivePing() {
  if (sock < 0) return false;

  uint8_t recvBuf[128];
  struct sockaddr_in addr;
  socklen_t addrLen = sizeof(addr);

  int received = recvfrom(sock, recvBuf, sizeof(recvBuf), 0,
                           (struct sockaddr*)&addr, &addrLen);

  if (received > 0) {
    // ICMP reply е на byte 20+ (след IP header)
    if (received >= 28 && recvBuf[20] == 0) { // type 0 = echo reply
      return true;
    }
  }
  return false;
}

void SpeedtestApp::update() {
  if (state == ST_DONE || state == ST_IDLE) return;

  unsigned long now = millis();

  // Анимация
  if (now - dotTimer >= 300) {
    dotTimer = now;
    dotCount = (dotCount + 1) % 4;
    redraw   = true;
  }

  if (state == ST_WAITING) {
    // Провери за отговор
    if (receivePing()) {
      results[currentHost].pingMs = (int)(millis() - pingStartTime);
      closeSocket();
      currentHost++;
      startNextPing();
      return;
    }

    // Timeout
    if (now - pingStartTime >= PING_TIMEOUT_MS) {
      results[currentHost].pingMs = -1;
      closeSocket();
      currentHost++;
      startNextPing();
    }
  }
}

void SpeedtestApp::handleInput(InputEvent event) {
  if (state == ST_DONE) {
    if (event == EVENT_RIGHT) {
      onEnter(); // Рестартирай
    } else if (event == EVENT_LEFT) {
      closeSocket();
      goBackApp();
    }
  } else {
    if (event == EVENT_LEFT) {
      closeSocket();
      goBackApp();
    }
  }
}

void SpeedtestApp::render() {
  Adafruit_SSD1306& display = displayManager->getDisplay();

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  // Header
  display.setCursor(0, 0);
  display.print("Ping Test");
  display.drawLine(0, 10, 127, 10, WHITE);

  // Резултати — всеки на отделен ред
  for (int i = 0; i < HOST_COUNT; i++) {
    int y = 13 + i * 12;

    display.setCursor(0, y);
    display.print(HOSTS[i]);

    display.setCursor(78, y);
    if (results[i].pingMs == -2) {
      // Pending — текущ хост показва анимация
      if (i == currentHost && state == ST_WAITING) {
        for (int d = 0; d < dotCount; d++) display.print(".");
      } else {
        display.print("---");
      }
    } else if (results[i].pingMs == -1) {
      display.print("timeout");
    } else {
      display.print(results[i].pingMs);
      display.print("ms");
    }
  }

  if (state == ST_DONE) {
    // Средно
    int total = 0, count = 0;
    for (int i = 0; i < HOST_COUNT; i++) {
      if (results[i].pingMs >= 0) {
        total += results[i].pingMs;
        count++;
      }
    }

    display.drawLine(0, 50, 127, 50, WHITE);
    display.setCursor(0, 53);
    if (count > 0) {
      display.print("Avg: ");
      display.print(total / count);
      display.print(" ms");
    } else {
      display.print("All timeouts");
    }

    display.setCursor(84, 53);
    display.print("R=retry");
  } else {
    display.setCursor(0, 57);
    display.print("LEFT = Cancel");
  }

  // БЕЗ display.display() — извиква се от AppManager
}

bool SpeedtestApp::needsRedraw() const { return redraw; }
void SpeedtestApp::clearRedrawFlag() { redraw = false; }
