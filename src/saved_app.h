#pragma once

#include "app.h"
#include "display_manager.h"
#include "nvs_manager.h"

enum SavedAppView {
  SAVED_LIST,
  SAVED_CONFIRM_DELETE
};

class SavedApp : public App {
  private:
  DisplayManager* displayManager;
  NVSManager*     nvsManager;
  bool redraw;

  SavedNetwork networks[MAX_SAVED_NETWORKS];
  int networkCount;

  int selectedIndex;
  int scrollOffset;
  int deleteIndex;       // индекс на мрежата за изтриване
  SavedAppView view;

  static const int VISIBLE_ITEMS = 4;

  void loadNetworks();
  void updateScroll();

  void renderList();
  void renderConfirm();

  public:
  SavedApp(DisplayManager* dm, NVSManager* nvs);

  void onEnter() override;
  void handleInput(InputEvent event) override;
  void render() override;
  void forceRedraw() override { redraw = true; }

  bool needsRedraw() const override;
  void clearRedrawFlag() override;
};
