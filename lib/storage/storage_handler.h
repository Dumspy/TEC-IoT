#ifndef STORAGE_HANDLER_H
#define STORAGE_HANDLER_H

#include <FS.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <vector>

class StorageHandler {
public:
    static StorageHandler& getInstance() {
        static StorageHandler instance;
        return instance;
    }

    void setupStorage();
    void logTemperatureToCSV(time_t timestamp, float temp);
    void addRowToCSV(const String& row);
    String getInitialDataJSON(int maxEntries = 100);
    void clearData();
    void removeRowByTimestamp(time_t timestamp);

private:
    StorageHandler() {}
    StorageHandler(const StorageHandler&) = delete;
    StorageHandler& operator=(const StorageHandler&) = delete;

    const char* csvFilePath = "/temperature_data.csv";
};

#endif
