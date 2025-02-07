#ifndef PREFERENCES_HANDLER_H
#define PREFERENCES_HANDLER_H

#include <Preferences.h>

Preferences preferences;

void initPreferences() {
    preferences.begin("wifi", false);
}

String getSSID() {
    return preferences.getString("ssid", "");
}

String getPassword() {
    return preferences.getString("password", "");
}

void saveSSID(const String& ssid) {
    preferences.putString("ssid", ssid);
}

void savePassword(const String& password) {
    preferences.putString("password", password);
}

void clearPreferences() {
    preferences.clear();
}

#endif
