#include "log_module.h"
#include <LittleFS.h>

LogModule::LogModule() : _lastLogTime(0) {}

void LogModule::begin() {
    if (!LittleFS.begin()) {
        Serial.println("[ERROR] LogModule: LittleFS Mount Failed!");
        return;
    }
    Serial.println("[INFO] LogModule: LittleFS Mounted Successfully.");
    writeHeaderIfNeeded();
}

void LogModule::writeHeaderIfNeeded() {
    File logFile = LittleFS.open(_logFilePath, "r");
    if (!logFile || logFile.size() == 0) {
        // File doesn't exist or is empty, write header
        logFile.close();
        logFile = LittleFS.open(_logFilePath, "w");
        if (logFile) {
            logFile.println("Timestamp,CurrentTemp,TargetTemp,PwmOutput,HeaterState");
            logFile.close();
            Serial.println("[INFO] LogModule: Log file header written.");
        } else {
            Serial.println("[ERROR] LogModule: Failed to open log file for writing header.");
        }
    } else {
        logFile.close();
    }
}

void LogModule::logData(float currentTemp, float targetTemp, float pwmOutput, int heaterState) {
    if (millis() - _lastLogTime < _logInterval) {
        return; // Not time to log yet
    }
    _lastLogTime = millis();

    File logFile = LittleFS.open(_logFilePath, "a"); // Open in append mode
    if (logFile) {
        // Auto-rotation: if file size exceeds maximum, clear it
        if (logFile.size() >= _maxLogSize) {
            logFile.close();
            Serial.println("[INFO] LogModule: Max log size reached. Initializing log.");
            clearLog(); // Clears file and writes header
            logFile = LittleFS.open(_logFilePath, "a"); // Reopen for appending new data
            if (!logFile) {
                Serial.println("[ERROR] LogModule: Failed to reopen after clearing.");
                return;
            }
        }

        String dataLine = String(millis()) + "," +
                          String(currentTemp, 2) + "," +
                          String(targetTemp, 2) + "," +
                  String(pwmOutput, 1) + "," +
                          String(heaterState);
        logFile.println(dataLine);
        logFile.close();
        // Serial.println("[INFO] LogModule: Data logged.");
    } else {
        Serial.println("[ERROR] LogModule: Failed to open log file for appending.");
    }
}

String LogModule::getLogFilePath() {
    return String(_logFilePath);
}

void LogModule::clearLog() {
    if (LittleFS.remove(_logFilePath)) {
        Serial.println("[INFO] LogModule: Log file cleared.");
        writeHeaderIfNeeded(); // Write header again after clearing
    } else {
        Serial.println("[ERROR] LogModule: Failed to clear log file.");
    }
}
