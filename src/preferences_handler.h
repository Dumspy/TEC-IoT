#ifndef PREFERENCES_HANDLER_H
#define PREFERENCES_HANDLER_H

#include <Preferences.h>

Preferences preferences; // Create a Preferences object

// Initialize the preferences with the namespace "wifi"
void initPreferences()
{
    preferences.begin("wifi", false); // false means read/write mode
}

// Get the stored SSID from preferences
// Returns: The stored SSID as a string
String getSSID()
{
    return preferences.getString("ssid", ""); // Return empty string if not found
}

// Get the stored password from preferences
// Returns: The stored password as a string
String getPassword()
{
    return preferences.getString("password", ""); // Return empty string if not found
}

// Save the SSID to preferences
// Parameters:
// - ssid: The SSID to be saved
void saveSSID(const String &ssid)
{
    preferences.putString("ssid", ssid); // Store the SSID
}

// Save the password to preferences
// Parameters:
// - password: The password to be saved
void savePassword(const String &password)
{
    preferences.putString("password", password); // Store the password
}

// Clear all preferences
void clearPreferences()
{
    preferences.clear(); // Clear all stored preferences
}

#endif
