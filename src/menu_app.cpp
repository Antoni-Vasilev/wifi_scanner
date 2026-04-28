#include "menu_app.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

MenuApp::MenuApp(Menu* rootMenu, DisplayManager* dm)
  : currentMenu(rootMenu),
    displayManager(dm),
    selectedIndex(0),
    scrollOffset(0),
    redraw(true) {}

void MenuApp::onEnter() {
  redraw = true;
}

void MenuApp::handleInput(InputEvent event) {
  switch (event) {
    case EVENT_UP:    moveUp();    break;
    case EVENT_DOWN:  moveDown();  break;
    case EVENT_LEFT:  moveLeft();  break;
    case EVENT_RIGHT: moveRight(); break;
    default: break;
  }
}

void MenuApp::moveUp() {
  if (currentMenu->itemCount <= 0) return;
  if (selectedIndex > 0) selectedIndex--;
  else selectedIndex = currentMenu->itemCount - 1;
  updateScroll();
  redraw = true;
}

void MenuApp::moveDown() {
  if (currentMenu->itemCount <= 0) return;
  if (selectedIndex < currentMenu->itemCount - 1) selectedIndex++;
  else selectedIndex = 0;
  updateScroll();
  redraw = true;
}

void MenuApp::moveLeft() {
  goBackMenu();
}

void MenuApp::moveRight() {
  selectItem();
}

void MenuApp::goBackMenu() {
  if (currentMenu->parent != nullptr) {
    currentMenu = currentMenu->parent;
    selectedIndex = 0;
    scrollOffset = 0;
    redraw = true;
  }
}

void MenuApp::selectItem() {
  if (currentMenu->itemCount <= 0) return;

  MenuItem& item = currentMenu->items[selectedIndex];

  switch (item.type) {
    case MENU_ACTION:
      if (item.action) item.action();
      break;
    case MENU_SUBMENU:
      if (item.submenu) {
        currentMenu = item.submenu;
        selectedIndex = 0;
        scrollOffset = 0;
        redraw = true;
      }
      break;
    case MENU_BACK:
      goBackMenu();
      break;
  }
}

void MenuApp::updateScroll() {
  if (selectedIndex < scrollOffset) {
    scrollOffset = selectedIndex;
  } else if (selectedIndex >= scrollOffset + VISIBLE_ITEMS) {
    scrollOffset = selectedIndex - VISIBLE_ITEMS + 1;
  }
}

void MenuApp::render() {
  Adafruit_SSD1306& display = displayManager->getDisplay();

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  char menuTitle[19];
  strncpy(menuTitle, currentMenu->title, 18);
  menuTitle[18] = '\0';
  display.setCursor(0, 0);
  display.print(menuTitle);
  display.drawLine(0, 10, 127, 10, WHITE);

  const int itemHeight = 10;
  const int startY = 14;

  int endIndex = scrollOffset + VISIBLE_ITEMS;
  if (endIndex > currentMenu->itemCount) endIndex = currentMenu->itemCount;

  for (int i = scrollOffset; i < endIndex; i++) {
    int visibleIndex = i - scrollOffset;
    int y = startY + visibleIndex * itemHeight;

    if (i == selectedIndex) {
      display.fillRect(0, y - 1, 128, 9, WHITE);
      display.setTextColor(BLACK);
    } else {
      display.setTextColor(WHITE);
    }

    display.setCursor(3, y);
    display.print(currentMenu->items[i].title);

    if (currentMenu->items[i].type == MENU_SUBMENU) {
      display.setCursor(120, y);
      display.print(">");
    }
  }

  if (currentMenu->itemCount > VISIBLE_ITEMS) {
    int barHeight = 40 / currentMenu->itemCount;
    if (barHeight < 4) barHeight = 4;
    int barY = 14 + (40 * scrollOffset) / currentMenu->itemCount;
    display.fillRect(124, barY, 3, barHeight, WHITE);
  }

  // БЕЗ display.display() — извиква се от AppManager
}

bool MenuApp::needsRedraw() const { return redraw; }
void MenuApp::clearRedrawFlag() { redraw = false; }