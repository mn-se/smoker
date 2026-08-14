#ifndef LOG_MODULE_H
#define LOG_MODULE_H

#include <LittleFS.h>
#include <Arduino.h>

class LogModule {
public:
    LogModule();
    void begin();
    void logData(float currentTemp, float targetTemp, float pwmOutput, int heaterState);
    String getLogFilePath();
    void clearLog();

private:
    const char* _logFilePath = "/smoker_log.csv";
    unsigned long _lastLogTime;
    const unsigned long _logInterval = 30000; // Log every 30 seconds
    const size_t _maxLogSize = 108000; // Max size: ~108KB (adjusted for 30s intervals)
    void writeHeaderIfNeeded();
};

#endif // LOG_MODULE_H
