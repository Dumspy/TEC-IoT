#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#define redLed 13
#define blueLed 12

const char* ssid = "e307";
const char* password = "rockyCartoon544";

const char* PARAM_INPUT_1 = "led";
const char* PARAM_INPUT_2 = "state";

AsyncWebServer server(80);

void updateLED(String led, String state) {
  int ledPin;
  if (led == "red") {
    ledPin = redLed;
  } else if (led == "blue") {
    ledPin = blueLed;
  } else {
    return;
  }
  Serial.println(ledPin);

  digitalWrite(ledPin, state == "true" ? HIGH : LOW);
}

void setup() {
  Serial.begin(9600);

  pinMode(redLed, OUTPUT);
  digitalWrite(redLed, LOW);
  pinMode(blueLed, OUTPUT);
  digitalWrite(blueLed, LOW);

  delay(5000);
  Serial.println("Booting...");

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi...");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnected to WiFi!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  pinMode(LED_BUILTIN, OUTPUT); // Initialize the LED pin as an output
  digitalWrite(LED_BUILTIN, HIGH); // Turn the LED on

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", "Hello, world");
  });

  server.on("/update", HTTP_PATCH, [] (AsyncWebServerRequest *request) {
    String inputMessage1;
    String inputMessage2;
    // GET input1 value on <ESP_IP>/update?led=<inputMessage1>&state=<inputMessage2>
    if (request->hasParam(PARAM_INPUT_1) && request->hasParam(PARAM_INPUT_2)) {
      inputMessage1 = request->getParam(PARAM_INPUT_1)->value();
      inputMessage2 = request->getParam(PARAM_INPUT_2)->value();

      updateLED(inputMessage1, inputMessage2);
    } 
    else {
      inputMessage1 = "No message sent";
      inputMessage2 = "No message sent";
    }
    Serial.print("LED: ");
    Serial.print(inputMessage1);
    Serial.print(" - Set to: ");
    Serial.println(inputMessage2);
    request->send(200, "text/plain", "OK");
  });

  server.begin();
}

void loop() {
}