#ifndef STORAGE_HANDLER_H
#define STORAGE_HANDLER_H

#include <FS.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <vector>

const char* csvFilePath = "/temperature_data.csv";

void setupStorage() {
    if (!SPIFFS.begin(true)) {
        Serial.println("An Error has occurred while mounting SPIFFS");
        return;
    }

    if (!SPIFFS.exists(csvFilePath)) {
        File file = SPIFFS.open(csvFilePath, FILE_WRITE);
        if (file) {
            file.println("timestamp;temp");
            file.close();
            Serial.println("Created temperature_data.csv with headers.");
        } else {
            Serial.println("Failed to create temperature_data.csv");
        }
    }
}


void logTemperatureToCSV(time_t timestamp, float temp) {
    delay(50); // Ensure previous file operations are completed
    File file = SPIFFS.open(csvFilePath, "a");
    if (!file) {
        Serial.println("Failed to open file for appending");
        return;
    }

    file.printf("%ld;%.2f\n", timestamp, temp);
    file.close();
}

void addRowToCSV(const String& row) {
    File file = SPIFFS.open(csvFilePath, "a");
    if (!file) return;

    file.println(row);
    file.close();
}

String getInitialDataJSON(int maxEntries = 100) { // Default: 100 entries
    delay(50); // Ensure previous file operations are completed
    Serial.println("getInitialDataJSON called");
    StaticJsonDocument<8192> doc;  // Increase buffer size if needed
    JsonArray history = doc.to<JsonArray>();

    // Check if the file exists
    if (!SPIFFS.exists(csvFilePath)) {
        Serial.println("Error: CSV file does not exist!");
        return "{}";  // Return empty JSON
    }

    File file = SPIFFS.open(csvFilePath, "r");
    if (!file) {
        Serial.println("Error: Failed to open CSV file!");
        return "{}";
    }

    // Use a circular buffer approach to store the last N entries
    String lines[maxEntries];  
    int count = 0;

    while (file.available()) {
        lines[count % maxEntries] = file.readStringUntil('\n');
        count++;
    }
    file.close();

    // Determine where to start reading
    int start = (count >= maxEntries) ? count % maxEntries : 0;
    int validEntries = min(count, maxEntries);

    Serial.printf("Reading %d entries starting from index %d\n", validEntries, start);
    Serial.println("Entries:");
    for (int i = 0; i < validEntries; i++) {
        int index = (start + i) % maxEntries;
        Serial.println(lines[index]);
        if (lines[index].length() > 0) {
            int sep = lines[index].indexOf(';');
            if (sep > 0) {
                JsonObject entry = history.createNestedObject();
                entry["timestamp"] = lines[index].substring(0, sep).toInt();
                entry["temp"] = lines[index].substring(sep + 1).toFloat();
                Serial.printf("Parsed entry: timestamp=%ld, temp=%.2f\n", entry["timestamp"].as<long>(), entry["temp"].as<float>());
            }
        }
    }

    // Serialize JSON
    String json;
    if (serializeJson(doc, json) == 0) {
        Serial.println("Error: Failed to serialize JSON!");
        return "{}";
    }

    Serial.println("Initial Data JSON: " + json);
    return json;
}

void clearData() {
    SPIFFS.remove(csvFilePath);
    setupStorage();
}

void removeRowByTimestamp(time_t timestamp) {
    if (!SPIFFS.exists(csvFilePath)) return;

    File file = SPIFFS.open(csvFilePath, "r");
    if (!file) return;

    std::vector<String> lines;
    while (file.available()) {
        lines.push_back(file.readStringUntil('\n'));
    }
    file.close();

    file = SPIFFS.open(csvFilePath, "w");
    if (!file) return;

    for (const auto& line : lines) {
        int sep = line.indexOf(';');
        if (sep > 0) {
            time_t rowTimestamp = line.substring(0, sep).toInt();
            if (rowTimestamp != timestamp) {
                file.println(line);
            }
        }
    }
    file.close();
}

#endif
