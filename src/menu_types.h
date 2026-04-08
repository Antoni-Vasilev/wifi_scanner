#pragma once

struct Menu;

enum MenuItemType {
  MENU_ACTION,
  MENU_SUBMENU,
  MENU_BACK
};

struct MenuItem {
  const char* title;
  MenuItemType type;
  void (*action)();
  Menu* submenu;
};

struct Menu {
  const char* title;
  MenuItem* items;
  int itemCount;
  Menu* parent;
};