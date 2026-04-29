#include "nvs_manager.h"
#include <Arduino.h>

// NVS структура:
// namespace "wifi"
//   key "count" → int (брой мрежи)
//   key "ssid0" → string (SSID на мрежа 0)
//   key "pass0" → string (парола на мрежа 0)
//   key "ssid1" → string ...
//   ... до MAX_SAVED_NETWORKS

NVSManager::NVSManager() {}

bool NVSManager::saveNetwork(const char* ssid, const char* password) {
  prefs.begin("wifi", false);

  int count = prefs.getInt("count", 0);

  // Провери дали вече съществува
  for (int i = 0; i < count; i++) {
    char ssidKey[8];
    snprintf(ssidKey, sizeof(ssidKey), "ssid%d", i);

    char storedSsid[33];
    prefs.getString(ssidKey, storedSsid, sizeof(storedSsid));

    if (strcmp(storedSsid, ssid) == 0) {
      // Обнови паролата
      char passKey[8];
      snprintf(passKey, sizeof(passKey), "pass%d", i);
      prefs.putString(passKey, password);
      prefs.end();
      return true;
    }
  }

  // Нова мрежа
  if (count >= MAX_SAVED_NETWORKS) {
    prefs.end();
    return false; // Пълно
  }

  char ssidKey[8], passKey[8];
  snprintf(ssidKey, sizeof(ssidKey), "ssid%d", count);
  snprintf(passKey, sizeof(passKey), "pass%d", count);

  prefs.putString(ssidKey, ssid);
  prefs.putString(passKey, password);
  prefs.putInt("count", count + 1);

  prefs.end();
  return true;
}

bool NVSManager::deleteNetwork(const char* ssid) {
  prefs.begin("wifi", false);

  int count = prefs.getInt("count", 0);
  int foundIndex = -1;

  for (int i = 0; i < count; i++) {
    char ssidKey[8];
    snprintf(ssidKey, sizeof(ssidKey), "ssid%d", i);

    char storedSsid[33];
    prefs.getString(ssidKey, storedSsid, sizeof(storedSsid));

    if (strcmp(storedSsid, ssid) == 0) {
      foundIndex = i;
      break;
    }
  }

  if (foundIndex < 0) {
    prefs.end();
    return false;
  }

  // Измести всички след намерения с една позиция назад
  for (int i = foundIndex; i < count - 1; i++) {
    char ssidKeyFrom[8], passKeyFrom[8];
    char ssidKeyTo[8],   passKeyTo[8];

    snprintf(ssidKeyFrom, sizeof(ssidKeyFrom), "ssid%d", i + 1);
    snprintf(passKeyFrom, sizeof(passKeyFrom), "pass%d", i + 1);
    snprintf(ssidKeyTo,   sizeof(ssidKeyTo),   "ssid%d", i);
    snprintf(passKeyTo,   sizeof(passKeyTo),   "pass%d", i);

    char tmpSsid[33], tmpPass[65];
    prefs.getString(ssidKeyFrom, tmpSsid, sizeof(tmpSsid));
    prefs.getString(passKeyFrom, tmpPass, sizeof(tmpPass));
    prefs.putString(ssidKeyTo, tmpSsid);
    prefs.putString(passKeyTo, tmpPass);
  }

  // Изтрий последния
  char ssidKeyLast[8], passKeyLast[8];
  snprintf(ssidKeyLast, sizeof(ssidKeyLast), "ssid%d", count - 1);
  snprintf(passKeyLast, sizeof(passKeyLast), "pass%d", count - 1);
  prefs.remove(ssidKeyLast);
  prefs.remove(passKeyLast);

  prefs.putInt("count", count - 1);
  prefs.end();
  return true;
}

bool NVSManager::findNetwork(const char* ssid, SavedNetwork& out) {
  prefs.begin("wifi", true); // read-only

  int count = prefs.getInt("count", 0);

  for (int i = 0; i < count; i++) {
    char ssidKey[8];
    snprintf(ssidKey, sizeof(ssidKey), "ssid%d", i);

    char storedSsid[33];
    prefs.getString(ssidKey, storedSsid, sizeof(storedSsid));

    if (strcmp(storedSsid, ssid) == 0) {
      char passKey[8];
      snprintf(passKey, sizeof(passKey), "pass%d", i);

      strncpy(out.ssid, storedSsid, 32);
      out.ssid[32] = '\0';
      prefs.getString(passKey, out.password, sizeof(out.password));

      prefs.end();
      return true;
    }
  }

  prefs.end();
  return false;
}

int NVSManager::getNetworks(SavedNetwork* out, int maxCount) {
  prefs.begin("wifi", true);

  int count = prefs.getInt("count", 0);
  if (count > maxCount) count = maxCount;

  for (int i = 0; i < count; i++) {
    char ssidKey[8], passKey[8];
    snprintf(ssidKey, sizeof(ssidKey), "ssid%d", i);
    snprintf(passKey, sizeof(passKey), "pass%d", i);

    prefs.getString(ssidKey, out[i].ssid,     sizeof(out[i].ssid));
    prefs.getString(passKey, out[i].password, sizeof(out[i].password));
  }

  prefs.end();
  return count;
}

int NVSManager::getNetworkCount() {
  prefs.begin("wifi", true);
  int count = prefs.getInt("count", 0);
  prefs.end();
  return count;
}

void NVSManager::clearAll() {
  prefs.begin("wifi", false);
  prefs.clear();
  prefs.end();
}
