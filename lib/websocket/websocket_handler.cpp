#include "websocket_handler.h"

void WebSocketHandler::setupWebSocket(AsyncWebServer& server) {
    ws.onEvent(onWebSocketEvent);
    server.addHandler(&ws);
}

void WebSocketHandler::handleWebSocket() {
    ws.cleanupClients();
}

void WebSocketHandler::sendTemperatureUpdate(const String& latestReading) {
    ws.textAll(latestReading);
}

void WebSocketHandler::onWebSocketEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len) {
    if (type == WS_EVT_CONNECT) {
        Serial.printf("WebSocket Client %u connected\n", client->id());
    } else if (type == WS_EVT_DISCONNECT) {
        Serial.printf("WebSocket Client %u disconnected\n", client->id());
    }
}
