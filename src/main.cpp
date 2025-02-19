#include <WiFi.h>
#include "time.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include "webserver_handler.h"
#include "preferences_handler.h"
#include "websocket_handler.h"
#include "storage_handler.h"

#define RESET_BUTTON_PIN 14
#define RESET_HOLD_TIME 10000

#define TEMPERATURE_PIN 4
OneWire oneWire(TEMPERATURE_PIN);
DallasTemperature sensors(&oneWire);

// Access Point default credentials
const char *apSSID = "ESP32_Felix";
const char *apPassword = "password";

// NTP settings
const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 0;
const int daylightOffset_sec = 0;

// Start the ESP32 in Access Point mode
void startAccessPoint()
{
  WiFi.softAP(apSSID, apPassword);
  Serial.println("Started Access Point: " + String(apSSID));
  Serial.println("Configure using 192.168.4.1");
}

// Variables to handle the reset button press
volatile unsigned long pressStartTime = 0; // Store the time the button was pressed
volatile bool buttonPressed = false;       // Flag to indicate if the button is pressed

// Interrupt Service Routine for the reset button
void IRAM_ATTR buttonISR()
{
  if (digitalRead(RESET_BUTTON_PIN) == LOW)
  {
    pressStartTime = millis(); // Record press time
    buttonPressed = true;      // Set flag
  }
  else
  {
    buttonPressed = false; // Reset flag when released
  }
}

// Enum to handle the current state of the ESP32
enum states
{
  AP,
  STA
}; // Access Point or Standard mode
enum states currentState = AP; // Default state is Access Point

// Function to sync time with NTP with retry mechanism
void syncTimeWithNTP()
{
  const int maxNTPRetries = 5;
  int retryCount = 0;
  bool timeSynced = false;

  while (retryCount < maxNTPRetries && !timeSynced)
  {
    if (WiFi.status() == WL_CONNECTED)
    {
      configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
      struct tm timeinfo;
      if (!getLocalTime(&timeinfo))
      {
        Serial.println("Failed to obtain time, retrying...");
        retryCount++;
        delay(2000); // Wait for 2 seconds before retrying
      }
      else
      {
        Serial.println("Time synchronized with NTP");
        timeSynced = true;
      }
    }
    else
    {
      Serial.println("Wi-Fi not connected, cannot sync time.");
      break;
    }
  }

  if (!timeSynced)
  {
    Serial.println("Failed to synchronize time with NTP after " + String(maxNTPRetries) + " attempts");
    Serial.println("Restarting...");
    ESP.restart();
  }
}

void setup()
{
  delay(5000); // Delay for 5 seconds to allow time to connect via serial

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  Serial.begin(115200);

  pinMode(RESET_BUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(RESET_BUTTON_PIN), buttonISR, CHANGE);

  initPreferences(); // Initialize preferences

  String savedSSID = getSSID();         // Get saved Wi-Fi SSID
  String savedPassword = getPassword(); // Get saved Wi-Fi Password

  currentState = savedSSID == "" ? AP : STA; // Set the current state based on saved Wi-Fi credentials

  setupStorage();

  // Switch case for setting up either Access Point or Standard mode
  switch (currentState)
  {
  case STA:
  {
    Serial.println("Connecting to Wi-Fi...");
    WiFi.begin(savedSSID.c_str(), savedPassword.c_str());

    int retryCount = 0;
    const int maxWifiRetries = 5;
    while (WiFi.status() != WL_CONNECTED && retryCount < maxWifiRetries)
    {
      Serial.println("Retrying Wi-Fi(" + savedSSID + ") connection...");
      delay(2000); // Wait for 2 seconds before retrying
      retryCount++;
    }

    if (WiFi.status() == WL_CONNECTED)
    {
      Serial.println("Connected! IP Address: " + WiFi.localIP().toString());
      syncTimeWithNTP();   // Sync time with NTP   // Initialize storage
      setupWebSocket();    // Initialize WebSocket
      setupSTAWebServer(); // Setup the web server for standard mode
    }
    else
    {
      Serial.println("Failed to connect to Wi-Fi after " + String(maxWifiRetries) + " attempts. Resetting and starting Access Point mode...");
      clearPreferences(); // Clear Wi-Fi credentials
      ESP.restart();      // Restart the ESP32
    }
  }
  break;

  case AP:
    Serial.println("Starting Access Point mode...");

    startAccessPoint(); // Start the ESP32 in Access Point mode
    setupAPWebServer(); // Setup the web server for AP mode
    break;
  }

  sensors.begin(); // Initialize temperature sensors
  server.begin();  // Start the web server
}

// Log the temperature reading to the CSV file and send it via WebSocket
void logTemperature()
{
  time_t now; // Declare a variable to store the current time
  time(&now); // Get the current time in seconds since Unix Epoch

  sensors.requestTemperatures();
  float temperatureC = sensors.getTempCByIndex(0);

  Serial.printf("%ld;%.2f\n", now, temperatureC);

  logTemperatureToCSV(now, temperatureC);                                                                // Log the temperature to the CSV file
  sendTemperatureUpdate("{\"timestamp\": " + String(now) + ", \"temp\": " + String(temperatureC) + "}"); // Send the temperature update via WebSocket
}

// Variables to handle the interval at which to log temperature
unsigned long previousTempMillis = 0;    // Store the last time the timestamp was printed
const long tempInterval = 1000 * 10 * 1; // Interval at which to log temperature (milliseconds)

// Variables to handle the interval at which to sync time with NTP
unsigned long previousSyncMillis = 0;     // Store the last time the time was synced
const long syncInterval = 1000 * 60 * 30; // Interval at which to sync time (milliseconds)

void loop()
{
  handleWebSocket(); // Handle WebSocket clients

  // Check if the reset button is pressed and held for the `RESET_HOLD_TIME` time
  if (buttonPressed && (millis() - pressStartTime >= RESET_HOLD_TIME))
  {
    Serial.println("Resetting Wi-Fi credentials...");
    for (int i = 0; i < 5; i++)
    {
      digitalWrite(LED_BUILTIN, LOW);
      delay(100);
      digitalWrite(LED_BUILTIN, HIGH);
      delay(100);
    }

    clearPreferences(); // Clear Wi-Fi credentials
    ESP.restart();      // Restart the ESP32
  }

  // Log temperature at the specified interval if in standard mode
  if (currentState == STA)
  {
    unsigned long currentMillis = millis();
    if (currentMillis - previousTempMillis >= tempInterval)
    {
      previousTempMillis = currentMillis;
      logTemperature();
    }

    // Sync time with NTP at the specified interval
    if (currentMillis - previousSyncMillis >= syncInterval)
    {
      previousSyncMillis = currentMillis;
      syncTimeWithNTP();
    }
  }
}