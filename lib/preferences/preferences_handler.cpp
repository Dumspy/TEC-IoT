#include "preferences_handler.h"

PreferencesHandler &PreferencesHandler::getInstance()
{
    static PreferencesHandler instance;
    return instance;
}

PreferencesHandler::PreferencesHandler()
{
    preferences.begin("wifi", false);
}

String PreferencesHandler::getSSID()
{
    return preferences.getString("ssid", "");
}

String PreferencesHandler::getPassword()
{
    return preferences.getString("password", "");
}

void PreferencesHandler::saveSSID(const String &ssid)
{
    preferences.putString("ssid", ssid);
}

void PreferencesHandler::savePassword(const String &password)
{
    preferences.putString("password", password);
}

void PreferencesHandler::clearPreferences()
{
    preferences.clear();
}
