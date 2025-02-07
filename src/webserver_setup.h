#ifndef WEBSERVER_SETUP_H
#define WEBSERVER_SETUP_H

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include "preferences_handler.h"
#include "websocket_handler.h"

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

void serveHTMLPage(AsyncWebServerRequest *request, String fileName) {
    File file = SPIFFS.open(fileName, "r");
    if (!file) {
        request->send(500, "text/plain", "Failed to open file");
        return;
    }

    String html = file.readString();
    file.close();
    request->send(200, "text/html", html);
}

void setupSTAWebServer() {
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", "Hello, World!");
    });

    server.on("/temp", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/temperature_data.csv", "text/csv");
    });
}

void setupAPWebServer() {
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        serveHTMLPage(request, "/ap_root.html");
    });

    server.on("/save", HTTP_POST, handleApSaveRequest);
}

#endif