#pragma once

#include <Preferences.h>

#define MAX_SAVED_NETWORKS 30

struct SavedNetwork {
  char ssid[33];
  char password[65];
};

class NVSManager {
  private:
  Preferences prefs;

  public:
  NVSManager();

  // Запази мрежа — ако вече съществува я обновява
  bool saveNetwork(const char* ssid, const char* password);

  // Изтрий мрежа по SSID
  bool deleteNetwork(const char* ssid);

  // Намери запазена мрежа по SSID — връща true ако е намерена
  bool findNetwork(const char* ssid, SavedNetwork& out);

  // Вземи всички запазени мрежи
  int getNetworks(SavedNetwork* out, int maxCount);

  // Брой запазени мрежи
  int getNetworkCount();

  // Изчисти всички
  void clearAll();
};
