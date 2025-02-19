#include "storage_handler.h"

void StorageHandler::setupStorage() {
    if (!SPIFFS.begin(true)) {
        Serial.println("An Error has occurred while mounting SPIFFS");
        return;
    }

    if (!SPIFFS.exists(csvFilePath)) {
        File file = SPIFFS.open(csvFilePath, FILE_WRITE);
        if (file) {
            file.println("timestamp;temp"); // Write CSV headers
            file.close();
            Serial.println("Created temperature_data.csv with headers.");
        } else {
            Serial.println("Failed to create temperature_data.csv");
        }
    }
}

void StorageHandler::logTemperatureToCSV(time_t timestamp, float temp) {
    delay(50); // Ensure previous file operations are completed
    File file = SPIFFS.open(csvFilePath, "a");
    if (!file) {
        Serial.println("Failed to open file for appending");
        return;
    }

    file.printf("%ld;%.2f\n", timestamp, temp); // Append the new data
    file.close();
}

void StorageHandler::addRowToCSV(const String& row) {
    File file = SPIFFS.open(csvFilePath, "a");
    if (!file)
        return;

    file.println(row); // Append the row
    file.close();
}

String StorageHandler::getInitialDataJSON(int maxEntries) {
    delay(50); // Ensure previous file operations are completed
    StaticJsonDocument<8192> doc; // Increase buffer size if needed
    JsonArray history = doc.to<JsonArray>();

    // Check if the file exists
    if (!SPIFFS.exists(csvFilePath)) {
        Serial.println("Error: CSV file does not exist!");
        return "{}"; // Return empty JSON
    }

    File file = SPIFFS.open(csvFilePath, "r");
    if (!file) {
        Serial.println("Error: Failed to open CSV file!");
        return "{}";
    }

    // Skip the header row
    file.readStringUntil('\n');

    // Use a circular buffer approach to store the last N entries
    String lines[maxEntries];
    int count = 0;

    while (file.available()) {
        lines[count % maxEntries] = file.readStringUntil('\n'); // Read each line
        count++;
    }
    file.close();

    // Determine where to start reading
    int start = (count >= maxEntries) ? count % maxEntries : 0;
    int validEntries = min(count, maxEntries);

    Serial.printf("Reading %d entries starting from index %d\n", validEntries, start);
    for (int i = 0; i < validEntries; i++) {
        int index = (start + i) % maxEntries;
        if (lines[index].length() > 0) {
            int sep = lines[index].indexOf(';');
            if (sep > 0) {
                JsonObject entry = history.createNestedObject();
                entry["timestamp"] = lines[index].substring(0, sep).toInt();
                entry["temp"] = lines[index].substring(sep + 1).toFloat();
            }
        }
    }

    // Serialize JSON
    String json;
    if (serializeJson(doc, json) == 0) {
        Serial.println("Error: Failed to serialize JSON!");
        return "{}";
    }
    return json;
}

void StorageHandler::clearData() {
    SPIFFS.remove(csvFilePath); // Remove the file
    setupStorage();             // Recreate the file with headers
}

void StorageHandler::removeRowByTimestamp(time_t timestamp) {
    if (!SPIFFS.exists(csvFilePath))
        return;

    File file = SPIFFS.open(csvFilePath, "r");
    if (!file)
        return;

    std::vector<String> lines;
    while (file.available()) {
        String line = file.readStringUntil('\n');
        line.trim(); // Remove any leading or trailing whitespace
        if (line.length() == 0) // Skip empty lines
            continue;

        if (line == "timestamp;temp") {
            continue;
        } else {
            int sep = line.indexOf(';');
            if (sep > 0) {
                time_t rowTimestamp = line.substring(0, sep).toInt();
                if (rowTimestamp != timestamp) {
                    lines.push_back(line); // Only keep lines that do not match the timestamp
                }
            }
        }
    }
    file.close();

    const char* tempFilePath = "/temp_temperature_data.csv";
    file = SPIFFS.open(tempFilePath, "w");
    if (!file)
        return;

    file.println("timestamp;temp"); // Write the header back
    for (const auto& line : lines) {
        if (line.length() > 0) // Ensure no empty lines are written
            file.println(line); // Write back all lines except the one to be removed
    }
    file.close();

    SPIFFS.remove(csvFilePath); // Remove the original file
    SPIFFS.rename(tempFilePath, csvFilePath); // Rename the temporary file to the original file
}
