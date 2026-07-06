/**
 * ESP32 Kindlelaser Max 260W - Laser Control Module
 * Controle de potência, segurança e monitoramento
 */

#ifndef LASER_CONTROL_H
#define LASER_CONTROL_H

#include <Arduino.h>
#include "config.h"

class LaserControl {
public:
    LaserControl();

    void begin();
    void update();

    // Power control
    void setPower(uint8_t percent);
    void adjustPower(int delta);
    uint8_t getPower() const;
    uint16_t getPowerWatts() const;

    // Speed control
    void setSpeed(uint8_t mmPerSec);
    void adjustSpeed(int delta);
    uint8_t getSpeed() const;

    // Laser enable/disable
    void laserOn();
    void laserOff();
    void emergencyStop();
    bool isLaserOn() const;
    void testPulse(uint16_t durationMs = 100);

    // Air assist
    void airAssistOn();
    void airAssistOff();
    void toggleAirAssist();
    bool isAirAssistOn() const;

    // Exhaust fan
    void exhaustOn();
    void exhaustOff();
    void toggleExhaust();
    bool isExhaustOn() const;

    // Safety monitoring
    bool isSafeToOperate() const;
    bool isWaterFlowOK() const;
    bool isLidClosed() const;
    bool isEstopOK() const;
    float getWaterTemp() const;
    bool isTempOK() const;
    bool isTempWarning() const;
    const char* getErrorMessage() const;

    // Material presets
    void loadPreset(uint8_t index);
    void nextPreset();
    void prevPreset();
    uint8_t getCurrentPresetIndex() const;
    const CuttingPreset& getCurrentPreset() const;

    // Mode
    LaserMode getMode() const;
    const char* getModeString() const;

    // Statistics
    unsigned long getCuttingTime() const;
    unsigned long getTotalCuttingTime() const;

private:
    void applyPWM();
    float readTemperature();
    void checkSafety();
    void setMode(LaserMode mode);

    uint8_t _power;             // Current power percentage
    uint8_t _speed;             // Current speed mm/s
    bool _laserEnabled;         // Laser output active
    bool _airAssistActive;      // Air assist solenoid
    bool _exhaustActive;        // Exhaust fan
    uint8_t _presetIndex;       // Current material preset

    LaserMode _mode;            // Operating mode
    char _errorMsg[64];         // Last error message

    // Safety state
    bool _waterFlowOK;
    bool _lidClosed;
    bool _estopOK;
    float _waterTemp;
    unsigned long _lastWaterPulse;
    unsigned long _lastSafetyCheck;

    // Timing
    unsigned long _cuttingStartTime;
    unsigned long _totalCuttingTime;
    unsigned long _warmupStart;
};

#endif
