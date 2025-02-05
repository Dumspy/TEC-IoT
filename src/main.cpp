#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Preferences.h>
#include <SPIFFS.h>
#include "time.h"
#include <OneWire.h>
#include <DallasTemperature.h>

#define RESET_BUTTON_PIN 14
#define RESET_HOLD_TIME 10000

Preferences preferences;
AsyncWebServer server(80);

#define TEMPERATURE_PIN 4     

OneWire oneWire(TEMPERATURE_PIN);
DallasTemperature sensors(&oneWire);

const char* apSSID = "ESP32_Felix"; // Name of ESP32 access point
const char* apPassword = "password"; // Password (at least 8 characters)

const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 0;
const int daylightOffset_sec = 0;

void startAccessPoint() {
    WiFi.softAP(apSSID, apPassword);
    Serial.println("Started Access Point: " + String(apSSID));
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

      configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

      // Wait for time to be set
      struct tm timeinfo;
      if (!getLocalTime(&timeinfo)) {
          Serial.println("Failed to obtain time");
          return;
      }

      if (!SPIFFS.exists("/temperature_data.csv")) {
        File file = SPIFFS.open("/temperature_data.csv", FILE_WRITE);
        if (file) {
          file.println("timestamp;temp");
          file.close();
          Serial.println("Created temperature_data.csv with headers.");
        } else {
          Serial.println("Failed to create temperature_data.csv");
        }
      }

      server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", "Hello, World!");
      });

      server.on("/temp", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/temperature_data.csv", "text/csv");
      });

      break;
    
    default:
      Serial.println("Starting Access Point mode...");
      startAccessPoint();

      server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        serveHTMLPage(request, "/ap_root.html");
      });
      server.on("/save", HTTP_POST, handleApSaveRequest);
      break;
  }

  sensors.begin();
  server.begin();
}

void logTemperatureToCSV() {
  time_t now;       // Declare a variable to store the current time
  time(&now);       // Get the current time in seconds since Unix Epoch

  sensors.requestTemperatures(); 
  float temperatureC = sensors.getTempCByIndex(0);

  Serial.printf("%ld;%.2f\n", now, temperatureC);

  File file = SPIFFS.open("/temperature_data.csv", FILE_APPEND);
  if (file) {
    file.printf("%ld;%.2f\n", now, temperatureC);
    file.close();
  } else {
    Serial.println("Failed to open temperature_data.csv for appending");
  }
}

unsigned long previousMillis = 0;  // Store the last time the timestamp was printed
const long interval = 10000;        // Interval at which to print the timestamp (1 second)


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

  if (currentState == STA){
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;
      logTemperatureToCSV();
    }
  }
}