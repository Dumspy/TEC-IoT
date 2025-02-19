#include "webserver_handler.h"
#include "preferences_handler.h"
#include "storage_handler.h"
#include "websocket_handler.h"
#include <SPIFFS.h>
#include <ArduinoJson.h>

void WebServerHandler::handleApSaveRequest(AsyncWebServerRequest *request) {
    String ssid = "";
    String password = "";

    if(request->hasParam("ssid", true) && request->hasParam("password", true)){
        ssid = request->getParam("ssid", true)->value();
        password = request->getParam("password", true)->value();
    }

    if (ssid != "" && password != "") {
        PreferencesHandler &preferences = PreferencesHandler::getInstance();
        preferences.saveSSID(ssid);
        preferences.savePassword(password);

        request->send(200, "text/plain", "Saved! Rebooting...");
        delay(1000);
        ESP.restart();
    } else {
        request->send(400, "text/plain", "Missing SSID or Password");
    }
}

void WebServerHandler::handleClearCSVRequest(AsyncWebServerRequest *request) {
    StorageHandler::getInstance().clearData();
    request->send(200, "text/plain", "CSV data cleared");
}

void WebServerHandler::handleAddRowRequest(AsyncWebServerRequest *request) {
    if (request->hasParam("row", true)) {
        String row = request->getParam("row", true)->value();
        StorageHandler::getInstance().addRowToCSV(row);
        request->send(200, "text/plain", "Row added");
    } else {
        request->send(400, "text/plain", "Missing row data");
    }
}

void WebServerHandler::handleDeleteRowRequest(AsyncWebServerRequest *request) {
    if (request->hasParam("timestamp", true)) {
        time_t timestamp = request->getParam("timestamp", true)->value().toInt();
        StorageHandler::getInstance().removeRowByTimestamp(timestamp);
        request->send(200, "text/plain", "Row deleted");
    } else {
        request->send(400, "text/plain", "Missing timestamp");
    }
}

void WebServerHandler::handleClearWifiRequest(AsyncWebServerRequest *request) {
    PreferencesHandler::getInstance().clearPreferences();
    request->send(200, "text/plain", "WiFi credentials cleared");
    ESP.restart();
}

void WebServerHandler::setupSTAWebServer() {
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/sta_root.html", "text/html");
    });

    server.on("/ping", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", "pong");
    });

    server.on("/temperature_data.csv", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/temperature_data.csv", "text/csv");
    });

    server.on("/sta_script.js", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/sta_script.js", "text/javascript");
    });

    server.on("/initial-data", HTTP_GET, [](AsyncWebServerRequest *request) {
        String jsonResponse = StorageHandler::getInstance().getInitialDataJSON();
        Serial.println(jsonResponse);
        request->send(200, "application/json", jsonResponse);
    });

    server.on("/clear-csv", HTTP_POST, handleClearCSVRequest);
    server.on("/add-row", HTTP_POST, handleAddRowRequest);
    server.on("/delete-row", HTTP_POST, handleDeleteRowRequest);
    server.on("/clear-wifi", HTTP_POST, handleClearWifiRequest);

    WebSocketHandler::getInstance().setupWebSocket(server);

    server.begin();
}

void WebServerHandler::setupAPWebServer() {
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/ap_root.html", "text/html");
    });

    server.on("/ap_script.js", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/ap_script.js", "text/javascript");
    });

    server.on("/save", HTTP_POST, handleApSaveRequest);

    server.begin();
}
