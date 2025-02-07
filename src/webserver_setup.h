#ifndef WEBSERVER_SETUP_H
#define WEBSERVER_SETUP_H

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include "preferences_handler.h"
#include "websocket_handler.h"
#include "storage_handler.h"

AsyncWebServer server(80);

void handleApSaveRequest(AsyncWebServerRequest *request) {
    if (request->hasParam("ssid", true) && request->hasParam("password", true)) {
        String ssid = request->getParam("ssid", true)->value();
        String password = request->getParam("password", true)->value();

        saveSSID(ssid);
        savePassword(password);

        request->send(200, "text/plain", "Saved! Rebooting...");
        delay(2000);
        ESP.restart();
    } else {
        request->send(400, "text/plain", "Missing SSID or Password");
    }
}

void handleClearCSVRequest(AsyncWebServerRequest *request) {
    clearData();
    request->send(200, "text/plain", "CSV data cleared");
}

void handleAddRowRequest(AsyncWebServerRequest *request) {
    if (request->hasParam("row", true)) {
        String row = request->getParam("row", true)->value();
        addRowToCSV(row);
        request->send(200, "text/plain", "Row added");
    } else {
        request->send(400, "text/plain", "Missing row data");
    }
}

void handleDeleteRowRequest(AsyncWebServerRequest *request) {
    if (request->hasParam("timestamp", true)) {
        time_t timestamp = request->getParam("timestamp", true)->value().toInt();
        removeRowByTimestamp(timestamp);
        request->send(200, "text/plain", "Row deleted");
    } else {
        request->send(400, "text/plain", "Missing timestamp");
    }
}

void setupSTAWebServer() {
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
        String jsonResponse = getInitialDataJSON();
        Serial.println(jsonResponse);
        request->send(200, "application/json", jsonResponse);
    });

    server.on("/clear-csv", HTTP_POST, handleClearCSVRequest);

    server.on("/add-row", HTTP_POST, handleAddRowRequest);

    server.on("/delete-row", HTTP_POST, handleDeleteRowRequest);
}

void setupAPWebServer() {
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/ap_root.html", "text/html");
    });

    server.on("/save", HTTP_POST, handleApSaveRequest);
}

#endif