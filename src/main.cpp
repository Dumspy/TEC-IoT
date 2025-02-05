#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Preferences.h> // To store Wi-Fi credentials
#include <SPIFFS.h>

#define RESET_BUTTON_PIN 14
#define RESET_HOLD_TIME 10000

Preferences preferences;
AsyncWebServer server(80);

const char* apSSID = "ESP32_Felix"; // Name of ESP32 access point
const char* apPassword = "password"; // Password (at least 8 characters)

void startAccessPoint() {
    WiFi.softAP(apSSID, apPassword);
    Serial.println("Started Access Point: " + String(apSSID));
}

void handleApRootRequest(AsyncWebServerRequest *request) {
    String html = R"rawliteral(
    <html><body>
    <h2>Wi-Fi Setup</h2>
    <form action="/save" method="POST">
      SSID: <input type="text" name="ssid"><br>
      Password: <input type="password" name="password"><br>
      <input type="submit" value="Save">
    </form>
    </body></html>
    )rawliteral";
    request->send(200, "text/html", html);
}

void handleApSaveRequest(AsyncWebServerRequest *request) {
    if (request->hasParam("ssid", true) && request->hasParam("password", true)) {
        String ssid = request->getParam("ssid", true)->value();
        String password = request->getParam("password", true)->value();

        preferences.putString("ssid", ssid);
        preferences.putString("password", password);

        request->send(200, "text/plain", "Saved! Rebooting...");
        delay(2000);
        ESP.restart();
    } else {
        request->send(400, "text/plain", "Missing SSID or Password");
    }
}

void handleHelloWorldRequest(AsyncWebServerRequest *request) {
    File file = SPIFFS.open("/hello.txt", "r");
    Serial.println("fileStatus:");
    Serial.println(file);

    if (!file) {
        request->send(500, "text/plain", "Failed to open file");
        return;
    }

    Serial.println(file.available());

    while(file.available()){
      Serial.write(file.read());
    }
    file.close();

    request->send(200, "text/plain", "ø");
}

volatile unsigned long pressStartTime = 0;  
volatile bool buttonPressed = false;

void IRAM_ATTR buttonISR() {
    if (digitalRead(RESET_BUTTON_PIN) == LOW) {  
        pressStartTime = millis();  // Record press time
        buttonPressed = true;  // Set flag
    } else {
        buttonPressed = false;  // Reset flag when released
    }
}

enum states {AP, STA};

enum states currentState = AP;

void setup() {
  delay(5000); // Delay for 5 seconds to allow time to connect via serial

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  Serial.begin(115200);
  preferences.begin("wifi", false); // Open Preferences

  pinMode(RESET_BUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(RESET_BUTTON_PIN), buttonISR, CHANGE);

  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  String savedSSID = preferences.getString("ssid", "");
  String savedPassword = preferences.getString("password", "");

  currentState = savedSSID == "" ? AP : STA;

  switch (currentState)
  {
    case STA:
      Serial.println("Connecting to Wi-Fi...");
      WiFi.begin(savedSSID.c_str(), savedPassword.c_str());

      if (WiFi.waitForConnectResult() == WL_CONNECTED) {
        Serial.println("Connected! IP Address: " + WiFi.localIP().toString());
      }

      server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", "Hello, World!");
      });

      server.on("/hello", HTTP_GET, handleHelloWorldRequest);

      break;
    
    default:
      Serial.println("Starting Access Point mode...");
      startAccessPoint();

      server.on("/", HTTP_GET, handleApRootRequest);
      server.on("/save", HTTP_POST, handleApSaveRequest);
      break;
  }

  server.begin();
}

void loop() {
  if (buttonPressed && (millis() - pressStartTime >= RESET_HOLD_TIME)) {
    Serial.println("Resetting Wi-Fi credentials...");

    for (int i = 0; i < 5; i++) {
      digitalWrite(LED_BUILTIN, LOW);
      delay(100);
      digitalWrite(LED_BUILTIN, HIGH);
      delay(100);
    }

    preferences.clear();
    ESP.restart();
  }
}