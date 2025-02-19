#ifndef PREFERENCES_HANDLER_H
#define PREFERENCES_HANDLER_H

#include <Preferences.h>

class PreferencesHandler
{
public:
    static PreferencesHandler &getInstance();
    String getSSID();
    String getPassword();
    void saveSSID(const String &ssid);
    void savePassword(const String &password);
    void clearPreferences();

private:
    PreferencesHandler();
    Preferences preferences;
};

#endif
