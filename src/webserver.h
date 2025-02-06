#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

AsyncWebServer server(80);

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

void setupApWebServer() {
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        serveHTMLPage(request, "/ap_root.html");
    });

    server.on("/save", HTTP_POST, handleApSaveRequest);
}

#endif