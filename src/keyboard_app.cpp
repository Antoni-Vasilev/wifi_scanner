#include "keyboard_app.h"
#include <Adafruit_SSD1306.h>

extern void goBackApp();
extern void openConnectApp(const char* ssid, const char* password);

const char KeyboardApp::SYMBOLS[] =
  "abcdefghijklmnopqrstuvwxyz"
  "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
  "0123456789"
  "!@#$%^&*()_+-=[]{}|;':\",./<>?`~ ";

KeyboardApp::KeyboardApp(DisplayManager* dm)
  : displayManager(dm),
    redraw(true),
    passwordLen(0),
    symbolIndex(0),
    onConfirm(nullptr) {
  ssid[0]     = '\0';
  password[0] = '\0';
}

void KeyboardApp::setTarget(const char* targetSsid, KeyboardCallback callback) {
  strncpy(ssid, targetSsid, 32);
  ssid[32] = '\0';
  onConfirm = callback;
}

void KeyboardApp::onEnter() {
  password[0] = '\0';
  passwordLen = 0;
  symbolIndex = 0;
  redraw = true;
}

void KeyboardApp::handleInput(InputEvent event) {
  bool isDel = (symbolIndex == SYMBOL_COUNT_VAL);
  bool isOk  = (symbolIndex == SYMBOL_COUNT_VAL + 1);

  switch (event) {
    case EVENT_UP:
      symbolIndex--;
      if (symbolIndex < 0) symbolIndex = TOTAL_ITEMS - 1;
      redraw = true;
      break;

    case EVENT_DOWN:
      symbolIndex++;
      if (symbolIndex >= TOTAL_ITEMS) symbolIndex = 0;
      redraw = true;
      break;

    case EVENT_RIGHT:
      if (isDel) {
        if (passwordLen > 0) {
          password[--passwordLen] = '\0';
          redraw = true;
        }
      } else if (isOk) {
        if (onConfirm) {
          onConfirm(ssid, password);
        }
      } else {
        if (passwordLen < MAX_PASSWORD_LEN) {
          password[passwordLen++] = SYMBOLS[symbolIndex];
          password[passwordLen]   = '\0';
          redraw = true;
        }
      }
      break;

    case EVENT_LEFT:
      if (passwordLen > 0) {
        password[--passwordLen] = '\0';
        redraw = true;
      } else {
        goBackApp();
      }
      break;

    default:
      break;
  }
}

void KeyboardApp::render() {
  Adafruit_SSD1306& display = displayManager->getDisplay();

  bool isDel = (symbolIndex == SYMBOL_COUNT_VAL);
  bool isOk  = (symbolIndex == SYMBOL_COUNT_VAL + 1);

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  // Header — SSID
  char header[19];
  strncpy(header, ssid, 18);
  header[18] = '\0';
  display.setCursor(0, 0);
  display.print(header);
  display.drawLine(0, 10, 127, 10, WHITE);

  // Паролата
  display.setCursor(0, 13);
  display.print("Pass: ");
  int showFrom = passwordLen > 12 ? passwordLen - 12 : 0;
  display.print(password + showFrom);
  display.print("_");

  // Стрелка нагоре
  display.setCursor(56, 25);
  display.print("^");

  if (isDel || isOk) {
    // Специален бутон
    const char* label = isDel ? "[ DEL ]" : "[  OK  ]";
    display.drawRect(28, 32, 72, 12, WHITE);
    display.setCursor(33, 35);
    display.print(label);
  } else {
    // Голям символ в центъра
    display.setTextSize(2);
    char sym[2] = { SYMBOLS[symbolIndex], '\0' };
    display.setCursor(56, 30);
    display.print(sym);
    display.setTextSize(1);

    // Група индикация вдясно
    const char* group = "";
    if (symbolIndex < 26)      group = "a-z";
    else if (symbolIndex < 52) group = "A-Z";
    else if (symbolIndex < 62) group = "0-9";
    else                       group = "!@#";
    display.setCursor(85, 34);
    display.print(group);
  }

  // Стрелка надолу
  display.setCursor(56, 47);
  display.print("v");

  // Hint долу
  display.setCursor(0, 57);
  display.print("R=add  L=del  OK=conn");

  // БЕЗ display.display() — извиква се от AppManager
}

bool KeyboardApp::needsRedraw() const {
  return redraw;
}

void KeyboardApp::clearRedrawFlag() {
  redraw = false;
}