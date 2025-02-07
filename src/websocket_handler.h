#ifndef WEBSOCKET_HANDLER_H
#define WEBSOCKET_HANDLER_H

#include <ESPAsyncWebServer.h>

extern AsyncWebServer server;

AsyncWebSocket ws("/ws");

void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
    if (type == WS_EVT_CONNECT) {
        Serial.printf("WebSocket Client %u connected\n", client->id());
    } else if (type == WS_EVT_DISCONNECT) {
        Serial.printf("WebSocket Client %u disconnected\n", client->id());
    }
}

void setupWebSocket() {
    ws.onEvent(onWebSocketEvent);
    server.addHandler(&ws);
}

void handleWebSocket() {
    ws.cleanupClients();
}

void sendTemperatureUpdate(String lastestReading) {
    ws.textAll(lastestReading);
}

#endif