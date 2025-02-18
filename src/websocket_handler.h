#ifndef WEBSOCKET_HANDLER_H
#define WEBSOCKET_HANDLER_H

#include <ESPAsyncWebServer.h>

extern AsyncWebServer server; // Declare the external AsyncWebServer object

AsyncWebSocket ws("/ws"); // Create a WebSocket object on the "/ws" endpoint

// Debugging function to print certain events to the serial monitor
void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
    if (type == WS_EVT_CONNECT) {
        Serial.printf("WebSocket Client %u connected\n", client->id()); // Log client connection
    } else if (type == WS_EVT_DISCONNECT) {
        Serial.printf("WebSocket Client %u disconnected\n", client->id()); // Log client disconnection
    }
}

// Setup the WebSocket server
void setupWebSocket() {
    // ws.onEvent(onWebSocketEvent); // Uncomment to enable WebSocket event handling
    server.addHandler(&ws); // Add the WebSocket handler to the server
}

// Handle cleanup of WebSocket clients
void handleWebSocket() {
    ws.cleanupClients(); // Clean up disconnected clients
}

// Send the latest temperature reading to all connected WebSocket clients
// Parameters:
// - lastestReading: The latest temperature reading as a string in JSON format
void sendTemperatureUpdate(String lastestReading) {
    ws.textAll(lastestReading); // Send the temperature update to all clients
}

#endif