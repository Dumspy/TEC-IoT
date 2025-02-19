#ifndef WEBSERVER_HANDLER_H
#define WEBSERVER_HANDLER_H

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

class WebServerHandler {
public:
    static WebServerHandler& getInstance() {
        static WebServerHandler instance;
        return instance;
    }

    void setupSTAWebServer();
    void setupAPWebServer();

private:
    WebServerHandler() : server(80) {}
    WebServerHandler(const WebServerHandler&) = delete;
    WebServerHandler& operator=(const WebServerHandler&) = delete;

    AsyncWebServer server;

    static void handleApSaveRequest(AsyncWebServerRequest *request);
    static void handleClearCSVRequest(AsyncWebServerRequest *request);
    static void handleAddRowRequest(AsyncWebServerRequest *request);
    static void handleDeleteRowRequest(AsyncWebServerRequest *request);
    static void handleClearWifiRequest(AsyncWebServerRequest *request);
};

#endif
