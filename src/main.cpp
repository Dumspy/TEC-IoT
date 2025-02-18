#include <WiFi.h>
#include "time.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include "webserver_setup.h"
#include "preferences_handler.h"
#include "websocket_handler.h"
#include "storage_handler.h"

#define RESET_BUTTON_PIN 14
#define RESET_HOLD_TIME 10000

#define TEMPERATURE_PIN 4     
OneWire oneWire(TEMPERATURE_PIN);
DallasTemperature sensors(&oneWire);

const char* apSSID = "ESP32_Felix";
const char* apPassword = "password";

const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 0;
const int daylightOffset_sec = 0;

void startAccessPoint() {
    WiFi.softAP(apSSID, apPassword);
    Serial.println("Started Access Point: " + String(apSSID));
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

  pinMode(RESET_BUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(RESET_BUTTON_PIN), buttonISR, CHANGE);

  initPreferences(); // Initialize preferences

  String savedSSID = getSSID();
  String savedPassword = getPassword();

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

      setupStorage();
      setupWebSocket();
      setupSTAWebServer();

      break;
    
    default:
      Serial.println("Starting Access Point mode...");
      
      startAccessPoint();
      setupAPWebServer();
      break;
  }

  sensors.begin();
  server.begin();
}

void logTemperature() {
  time_t now;       // Declare a variable to store the current time
  time(&now);       // Get the current time in seconds since Unix Epoch

  sensors.requestTemperatures(); 
  float temperatureC = sensors.getTempCByIndex(0);

  Serial.printf("%ld;%.2f\n", now, temperatureC);

  logTemperatureToCSV(now, temperatureC);
  sendTemperatureUpdate("{\"timestamp\": " + String(now) + ", \"temp\": " + String(temperatureC) + "}");
}

unsigned long previousMillis = 0;  // Store the last time the timestamp was printed
const long interval = 1000 * 60 * 5;        // Interval at which to log temperature (milliseconds)

void loop() {
  handleWebSocket();

  if (buttonPressed && (millis() - pressStartTime >= RESET_HOLD_TIME)) {
    Serial.println("Resetting Wi-Fi credentials...");

    for (int i = 0; i < 5; i++) {
      digitalWrite(LED_BUILTIN, LOW);
      delay(100);
      digitalWrite(LED_BUILTIN, HIGH);
      delay(100);
    }

    clearPreferences();
    ESP.restart();
  }

  if (currentState == STA){
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;
      logTemperature();
    }
  }
}