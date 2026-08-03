#include "temp_control.h"
#include "../../include/config.h"
#include "AppConfig.h"

TempControlModule::TempControlModule(MAX6675Module& max6675, MerossControlModule& meross, LogModule& logModule)
    : _max6675(max6675), _meross(meross), _logModule(logModule), _state(IDLE), _targetTemp(0), _lastUpdate(0), _heaterState(-1),
      _Kp(PID_KP), _Ki(PID_KI), _Kd(PID_KD), _integral(0), _lastError(0), _pidOutput(0), _windowSize(PWM_WINDOW_SIZE_MS), _windowStartTime(0) {
    // Override with saved config values if available
    AppConfig::readPIDConfig(_Kp, _Ki, _Kd, _windowSize);
}

void TempControlModule::setTargetTemp(float temp) {
    _targetTemp = temp;
    // Reset integral and last error to avoid wind-up / large overshoot
    _integral = 0;
    _lastError = 0;
    _pidOutput = 0;
}

void TempControlModule::setPIDConfig(float kp, float ki, float kd, unsigned long windowSize) {
    _Kp = kp;
    _Ki = ki;
    _Kd = kd;
    _windowSize = windowSize;
    _integral = 0; // Reset integral on parameter change
}

void TempControlModule::startTempControl() {
    if (_state == ERROR_STATE) return;
    _state = TEMP_CONTROL;
    _integral = 0;
    _lastError = 0;
    _windowStartTime = millis();
}

void TempControlModule::startForceOn() {
    if (_state == ERROR_STATE) return;
    _state = FORCE_ON;
    if (_meross.toggle(true)) {
        _heaterState = 1;
    }
}

void TempControlModule::startMonitoring() {
    if (_state == ERROR_STATE) return;
    _state = MONITORING;
}

void TempControlModule::stop() {
    _state = IDLE;
    if (_meross.toggle(false)) {
        _heaterState = 0;
    }
}

void TempControlModule::update() {
    if (_state == IDLE || millis() - _lastUpdate < TEMP_UPDATE_INTERVAL_MS) return;
    _lastUpdate = millis();

    float temp = _max6675.readTempC();
    if (temp < TEMP_ERROR_THRESHOLD) {
        Serial.println("[ERROR] Temperature sensor reading failed. SAFETY CUT-OFF!");
        _state = ERROR_STATE;
        if (_meross.toggle(false)) {
            _heaterState = 0;
        }
        _logModule.logData(temp, _targetTemp, _heaterState); // Log data even on error
        return; // Error
    }

    if (_state == ERROR_STATE) {
        _logModule.logData(temp, _targetTemp, _heaterState); // Log data in error state
        return; // Do nothing in error state
    }

    Serial.printf("[TEMP] Current: %.2f°C, Target: %.2f°C, State: ", temp, _targetTemp);

    if (_state == TEMP_CONTROL) {
        Serial.print("TEMP_CONTROL");
        // PID Calculation
        float error = _targetTemp - temp;
        float dt = TEMP_UPDATE_INTERVAL_MS / 1000.0f; // 秒単位に変換
        _integral += error * dt;
        
        // Anti-windup
        if (_integral > PID_INTEGRAL_CLAMP) _integral = PID_INTEGRAL_CLAMP;
        else if (_integral < -PID_INTEGRAL_CLAMP) _integral = -PID_INTEGRAL_CLAMP;

        float dError = (error - _lastError) / dt;
        _lastError = error;

        float output = (_Kp * error) + (_Ki * _integral) + (_Kd * dError);
        
        // Output normalization (0 to 100)
        if (output > 100.0) output = 100.0;
        else if (output < 0.0) output = 0.0;
        
        _pidOutput = output / 100.0;

        // PWM Control (Time-Proportioning)
        unsigned long now = millis();
        if (now - _windowStartTime >= _windowSize) {
            _windowStartTime = now;
        }
        
        unsigned long onTime = (unsigned long)(_windowSize * _pidOutput);
        bool shouldBeOn = (now - _windowStartTime < onTime);

        if (_pidOutput < 0.01) shouldBeOn = false;
        if (_pidOutput > 0.99) shouldBeOn = true;

        if (shouldBeOn) {
            if (_heaterState != 1) {
                if (_meross.toggle(true)) {
                    _heaterState = 1;
                    Serial.printf(" -> HEATER ON (PID: %.1f%%)\n", output);
                } else {
                    Serial.println(" -> HEATER ON FAILED");
                }
            } else {
                Serial.printf(" -> MAINTAINING ON (PID: %.1f%%)\n", output);
            }
        } else {
            if (_heaterState != 0) {
                if (_meross.toggle(false)) {
                    _heaterState = 0;
                    Serial.printf(" -> HEATER OFF (PID: %.1f%%)\n", output);
                } else {
                    Serial.println(" -> HEATER OFF FAILED");
                }
            } else {
                Serial.printf(" -> MAINTAINING OFF (PID: %.1f%%)\n", output);
            }
        }
    } else if (_state == FORCE_ON) {
        Serial.println("FORCE_ON");
    } else if (_state == MONITORING) {
        Serial.printf("MONITORING -> %.2f°C\n", temp);
    }
    _logModule.logData(temp, _targetTemp, _heaterState); // Log data at the end of update
}

ControlState TempControlModule::getState() {
    return _state;
}

float TempControlModule::getCurrentTemp() {
    return _max6675.readTempC();
}

int TempControlModule::getHeaterState() {
    return _heaterState;
}