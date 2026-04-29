#include "saved_app.h"
#include <Adafruit_SSD1306.h>

extern void goBackApp();
extern void connectToNetwork(const char* ssid, const char* password);

SavedApp::SavedApp(DisplayManager* dm, NVSManager* nvs)
  : displayManager(dm),
    nvsManager(nvs),
    redraw(true),
    networkCount(0),
    selectedIndex(0),
    scrollOffset(0),
    deleteIndex(-1),
    view(SAVED_LIST) {}

void SavedApp::onEnter() {
  selectedIndex = 0;
  scrollOffset  = 0;
  deleteIndex   = -1;
  view          = SAVED_LIST;
  loadNetworks();
  redraw = true;
}

void SavedApp::loadNetworks() {
  networkCount = nvsManager->getNetworks(networks, MAX_SAVED_NETWORKS);
}

void SavedApp::handleInput(InputEvent event) {
  if (view == SAVED_CONFIRM_DELETE) {
    switch (event) {
      case EVENT_RIGHT:
        // Потвърди изтриването
        nvsManager->deleteNetwork(networks[deleteIndex].ssid);
        loadNetworks();
        if (selectedIndex >= networkCount && selectedIndex > 0) {
          selectedIndex--;
        }
        updateScroll();
        view   = SAVED_LIST;
        redraw = true;
        break;

      case EVENT_LEFT:
        // Откажи — върни се към списъка
        view   = SAVED_LIST;
        redraw = true;
        break;

      default:
        break;
    }
    return;
  }

  // SAVED_LIST view
  int total = networkCount + 1;

  switch (event) {
    case EVENT_UP:
      if (selectedIndex > 0) selectedIndex--;
      else selectedIndex = total - 1;
      updateScroll();
      redraw = true;
      break;

    case EVENT_DOWN:
      if (selectedIndex < total - 1) selectedIndex++;
      else selectedIndex = 0;
      updateScroll();
      redraw = true;
      break;

    case EVENT_RIGHT:
      if (selectedIndex == networkCount) {
        // "Clear All" — покажи confirmation
        deleteIndex = -1; // -1 означава clear all
        view        = SAVED_CONFIRM_DELETE;
      } else {
        // Свържи се
        connectToNetwork(
          networks[selectedIndex].ssid,
          networks[selectedIndex].password
        );
      }
      redraw = true;
      break;

    case EVENT_RIGHT_LONG:
      // Long press — покажи confirmation за изтриване
      if (selectedIndex < networkCount) {
        deleteIndex = selectedIndex;
        view        = SAVED_CONFIRM_DELETE;
        redraw      = true;
      }
      break;

    case EVENT_LEFT:
      // Винаги излиза назад
      goBackApp();
      break;

    default:
      break;
  }
}

void SavedApp::updateScroll() {
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

void SavedApp::render() {
  if (view == SAVED_CONFIRM_DELETE) {
    renderConfirm();
  } else {
    renderList();
  }
}

void SavedApp::renderList() {
  Adafruit_SSD1306& display = displayManager->getDisplay();

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  display.setCursor(0, 0);
  display.print("Saved (");
  display.print(networkCount);
  display.print(")");
  display.drawLine(0, 10, 127, 10, WHITE);

  if (networkCount == 0) {
    display.setCursor(0, 24);
    display.println("No saved networks.");
    display.setCursor(0, 57);
    display.print("LEFT = Back");
    return;
  }

  int total           = networkCount + 1;
  const int startY    = 13;
  const int itemH     = 10;
  const int CONTENT_WIDTH = 120;
  const int SCROLLBAR_X   = 121;

  int endIndex = scrollOffset + VISIBLE_ITEMS;
  if (endIndex > total) endIndex = total;

  for (int i = scrollOffset; i < endIndex; i++) {
    int y           = startY + (i - scrollOffset) * itemH;
    bool isSelected = (i == selectedIndex);
    bool isClearAll = (i == networkCount);

    if (isSelected) {
      display.fillRect(0, y - 1, CONTENT_WIDTH, 9, WHITE);
      display.setTextColor(BLACK);
    } else {
      display.setTextColor(WHITE);
    }

    display.setCursor(3, y);

    if (isClearAll) {
      display.print("Clear All");
    } else {
      char label[19];
      strncpy(label, networks[i].ssid, 18);
      label[18] = '\0';
      if (strlen(networks[i].ssid) > 18) {
        label[15] = '.';
        label[16] = '.';
        label[17] = '.';
      }
      display.print(label);
    }
  }

  display.setTextColor(WHITE);

  if (total > VISIBLE_ITEMS) {
    int barHeight = (VISIBLE_ITEMS * 40) / total;
    if (barHeight < 4) barHeight = 4;
    int barY = 13 + (scrollOffset * 40) / total;
    display.fillRect(SCROLLBAR_X, barY, 3, barHeight, WHITE);
  }

  display.setCursor(0, 57);
  if (selectedIndex == networkCount) {
    display.print("R=clear all  L=back");
  } else {
    display.print("R=connect  hold R=del");
  }
}

void SavedApp::renderConfirm() {
  Adafruit_SSD1306& display = displayManager->getDisplay();

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  display.setCursor(0, 0);
  display.print("Delete?");
  display.drawLine(0, 10, 127, 10, WHITE);

  // SSID
  display.setCursor(0, 14);
  if (deleteIndex == -1) {
    display.print("All saved networks");
  } else {
    char label[19];
    strncpy(label, networks[deleteIndex].ssid, 18);
    label[18] = '\0';
    if (strlen(networks[deleteIndex].ssid) > 18) {
      label[15] = '.';
      label[16] = '.';
      label[17] = '.';
    }
    display.print(label);
  }

  display.drawLine(0, 36, 127, 36, WHITE);

  display.setCursor(0, 40);
  display.print("RIGHT - Confirm");

  display.setCursor(0, 52);
  display.print("LEFT  - Cancel");
}

bool SavedApp::needsRedraw() const { return redraw; }
void SavedApp::clearRedrawFlag() { redraw = false; }