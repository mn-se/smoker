#ifndef TEMP_CONTROL_MODULE_H
#define TEMP_CONTROL_MODULE_H

#include "max6675.h"
#include "meross_control.h"
#include "../core/log_module.h"

enum ControlState { IDLE, TEMP_CONTROL, FORCE_ON, MONITORING, ERROR_STATE };

#include "../../include/config.h"

class TempControlModule {
public:
    TempControlModule(MAX6675Module& max6675, MerossControlModule& meross, LogModule& logModule);
    void setTargetTemp(float temp);
    void setPIDConfig(float kp, float ki, float kd, unsigned long windowSize);
    void startTempControl();
    void startForceOn();
    void startMonitoring();
    void stop();
    void update();
    ControlState getState();
    float getCurrentTemp();
    unsigned long getLastSensorReadMs() { return _lastUpdate; }
    float getTargetTemp() { return _targetTemp; }
    float getKp() { return _Kp; }
    float getKi() { return _Ki; }
    float getKd() { return _Kd; }
    unsigned long getWindowSize() { return _windowSize; }
    int getHeaterState();
private:
    MAX6675Module& _max6675;
    MerossControlModule& _meross;
    LogModule& _logModule; // Reference to LogModule
    ControlState _state;
    float _currentTemp;
    float _targetTemp;
    unsigned long _lastUpdate;
    int _heaterState;

    // PID Control variables
    float _Kp, _Ki, _Kd;
    float _integral;
    float _lastError;
    float _pidOutput; // 0.0 to 1.0

    // Time-Proportioning (PWM) control variables
    unsigned long _windowSize; // e.g., 120000 ms (2 minutes)
    unsigned long _windowStartTime;
};

#endif