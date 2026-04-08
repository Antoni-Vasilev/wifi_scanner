#pragma once

#include "app.h"
#include "menu_types.h"
#include "display_manager.h"

class MenuApp : public App {
  private:
  Menu* currentMenu;
  DisplayManager* displayManager;

  int selectedIndex;
  int scrollOffset;
  bool redraw;

  static const int VISIBLE_ITEMS = 5;

  void moveUp();
  void moveDown();
  void moveLeft();
  void moveRight();
  void goBackMenu();
  void selectItem();
  void updateScroll();

  public:
  MenuApp(Menu* rootMenu, DisplayManager* dm);

  void onEnter() override;
  void handleInput(InputEvent event) override;
  void render() override;

  bool needsRedraw() const override;
  void clearRedrawFlag() override;
};