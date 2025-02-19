#ifndef WEBSOCKET_HANDLER_H
#define WEBSOCKET_HANDLER_H

#include <ESPAsyncWebServer.h>

class WebSocketHandler {
public:
    static WebSocketHandler& getInstance() {
        static WebSocketHandler instance;
        return instance;
    }

    void setupWebSocket(AsyncWebServer& server);
    void handleWebSocket();
    void sendTemperatureUpdate(const String& latestReading);

private:
    WebSocketHandler() : ws("/ws") {}
    WebSocketHandler(const WebSocketHandler&) = delete;
    WebSocketHandler& operator=(const WebSocketHandler&) = delete;

    AsyncWebSocket ws;

    static void onWebSocketEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len);
};

#endif
