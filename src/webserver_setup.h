#ifndef WEBSERVER_SETUP_H
#define WEBSERVER_SETUP_H

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include "preferences_handler.h"
#include "websocket_handler.h"
#include "storage_handler.h"

AsyncWebServer server(80); // Create an AsyncWebServer object on port 80

// Handle the request to save the SSID and password
void handleApSaveRequest(AsyncWebServerRequest *request)
{
    if (request->hasParam("ssid", true) && request->hasParam("password", true))
    {
        // Get the SSID and password from the request
        String ssid = request->getParam("ssid", true)->value();
        String password = request->getParam("password", true)->value();

        // Save the SSID and password to preferences
        saveSSID(ssid);
        savePassword(password);

        request->send(200, "text/plain", "Saved! Rebooting...");
        delay(2000);   // Delay to allow the response to be sent before rebooting
        ESP.restart(); // Restart the ESP32
    }
    else
    {
        request->send(400, "text/plain", "Missing SSID or Password");
    }
}

// Handle the request to clear the CSV data
void handleClearCSVRequest(AsyncWebServerRequest *request)
{
    clearData(); // Clear the CSV data
    request->send(200, "text/plain", "CSV data cleared");
}

// Handle the request to add a row to the CSV data
void handleAddRowRequest(AsyncWebServerRequest *request)
{
    if (request->hasParam("row", true))
    {
        String row = request->getParam("row", true)->value();
        addRowToCSV(row); // Add the row to the CSV data
        request->send(200, "text/plain", "Row added");
    }
    else
    {
        request->send(400, "text/plain", "Missing row data");
    }
}

// Handle the request to delete a row from the CSV data
void handleDeleteRowRequest(AsyncWebServerRequest *request)
{
    if (request->hasParam("timestamp", true))
    {
        time_t timestamp = request->getParam("timestamp", true)->value().toInt();
        removeRowByTimestamp(timestamp); // Remove the row with the specified timestamp
        request->send(200, "text/plain", "Row deleted");
    }
    else
    {
        request->send(400, "text/plain", "Missing timestamp");
    }
}

// Handle the request to clear the WiFi credentials
void handleClearWifiRequest(AsyncWebServerRequest *request)
{
    clearPreferences(); // Clear the WiFi credentials
    ESP.restart();      // Restart the ESP32
    request->send(200, "text/plain", "WiFi credentials cleared");
}

// Setup the web server for standard mode defining all routes needed
void setupSTAWebServer()
{
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              {
                  request->send(SPIFFS, "/sta_root.html", "text/html"); // Serve the root HTML page
              });

    server.on("/ping", HTTP_GET, [](AsyncWebServerRequest *request)
              {
                  request->send(200, "text/plain", "pong"); // Respond to ping requests
              });

    server.on("/temperature_data.csv", HTTP_GET, [](AsyncWebServerRequest *request)
              {
                  request->send(SPIFFS, "/temperature_data.csv", "text/csv"); // Serve the CSV file
              });

    server.on("/sta_script.js", HTTP_GET, [](AsyncWebServerRequest *request)
              {
                  request->send(SPIFFS, "/sta_script.js", "text/javascript"); // Serve the JavaScript file
              });

    server.on("/initial-data", HTTP_GET, [](AsyncWebServerRequest *request)
              {
                  String jsonResponse = getInitialDataJSON(); // Get the initial data in JSON format
                  Serial.println(jsonResponse);
                  request->send(200, "application/json", jsonResponse); // Serve the JSON response
              });

    server.on("/clear-csv", HTTP_POST, handleClearCSVRequest); // Handle requests to clear the CSV data

    server.on("/add-row", HTTP_POST, handleAddRowRequest); // Handle requests to add a row to the CSV data

    server.on("/delete-row", HTTP_POST, handleDeleteRowRequest); // Handle requests to delete a row from the CSV data

    server.on("/clear-wifi", HTTP_POST, handleClearWifiRequest); // Handle requests to clear the WiFi credentials
}

// Setup the web server for access point mode defining all routes needed
void setupAPWebServer()
{
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              {
                  request->send(SPIFFS, "/ap_root.html", "text/html"); // Serve the root HTML page for AP mode
              });

    server.on("/ap_script.js", HTTP_GET, [](AsyncWebServerRequest *request)
              {
                  request->send(SPIFFS, "/ap_script.js", "text/javascript"); // Serve the JavaScript file
              });

    server.on("/save", HTTP_POST, handleApSaveRequest); // Handle requests to save the SSID and password
}

#endif