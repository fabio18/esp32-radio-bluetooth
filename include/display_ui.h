/**
 * ESP32 Kindlelaser Max 260W - Display UI
 * ILI9341 TFT 240x320 + XPT2046 Touch
 * Interface industrial para controle de corte de metal
 */

#ifndef DISPLAY_UI_H
#define DISPLAY_UI_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include "config.h"

struct TouchArea {
    int16_t x, y, w, h;
    TouchButton id;
};

class DisplayUI {
public:
    DisplayUI();

    void begin();
    void calibrateTouch();
    void drawSplashScreen();

    // Main screen
    void drawMainScreen(uint8_t power, uint16_t powerWatts, uint8_t speed,
                        const CuttingPreset& preset, uint8_t presetIndex,
                        bool laserOn, bool airOn, bool exhaustOn,
                        LaserMode mode);

    // Partial updates
    void updatePower(uint8_t power, uint16_t powerWatts);
    void updateSpeed(uint8_t speed);
    void updateLaserStatus(bool laserOn, LaserMode mode);
    void updateAirStatus(bool airOn);
    void updateExhaustStatus(bool exhaustOn);
    void updatePreset(const CuttingPreset& preset, uint8_t presetIndex);
    void updateCuttingTime(unsigned long ms);
    void updateSafety(bool waterOK, bool lidClosed, bool estopOK,
                      float waterTemp, bool tempWarning);

    // Touch
    TouchButton checkTouch();

    // Notifications
    void showMessage(const char* msg, uint16_t color = COLOR_TEXT);
    void showError(const char* msg);

private:
    TFT_eSPI _tft;
    TFT_eSprite _spritePower;
    TFT_eSprite _spriteStatus;
    unsigned long _lastTouchTime;

    // Cached state
    uint8_t _lastPower;
    uint8_t _lastSpeed;
    bool _lastLaserOn;
    LaserMode _lastMode;

    // Touch areas
    static const int MAX_BUTTONS = 12;
    TouchArea _buttons[MAX_BUTTONS];
    int _buttonCount;

    // Drawing helpers
    void drawHeader();
    void drawPowerSection(uint8_t power, uint16_t watts);
    void drawSpeedSection(uint8_t speed);
    void drawPresetSection(const CuttingPreset& preset, uint8_t index);
    void drawControls(bool laserOn, bool airOn, bool exhaustOn);
    void drawSafetyBar(bool waterOK, bool lidClosed, bool estopOK, float temp);
    void drawPowerBar(uint8_t power);
    void drawGlowLine(int y, uint16_t color);

    // Touch buttons
    void drawButton(int16_t x, int16_t y, int16_t w, int16_t h,
                    const char* label, uint16_t glowColor, bool active = false);
    void drawLaserButton(bool laserOn);

    // Button management
    void registerButton(int16_t x, int16_t y, int16_t w, int16_t h, TouchButton id);
    void clearButtons();
    bool getTouchPoint(int16_t &x, int16_t &y);
    TouchButton hitTest(int16_t x, int16_t y);
};

#endif
