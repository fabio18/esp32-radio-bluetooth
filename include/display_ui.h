/**
 * ESP32 Radio Bluetooth - Display UI Pioneer Style
 * ILI9341 TFT 240x320 + XPT2046 Touch
 * Visual automotivo estilo Pioneer/Kenwood
 */

#ifndef DISPLAY_UI_H
#define DISPLAY_UI_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include "config.h"
#include "TEA5767.h"

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

    // Main screens
    void drawFMScreen(float frequency, bool stereo, uint8_t signalLevel,
                      bool muted, int presetIndex, int presetCount, uint8_t volume,
                      const char* ledEffectName = "VU METER");
    void drawBluetoothScreen(bool connected, const char* deviceName,
                              bool muted, uint8_t volume, bool isPlaying,
                              const char* ledEffectName = "VU METER");

    // Partial updates
    void updateFrequency(float frequency);
    void updateSignalLevel(uint8_t level);
    void updateStereoIndicator(bool stereo);
    void updateMuteIndicator(bool muted);
    void updateVolume(uint8_t volume);
    void updatePresetInfo(int presetIndex, float presetFreq, int presetCount);
    void updateBTStatus(bool connected, const char* deviceName);
    void updatePlayStatus(bool isPlaying);

    // EQ Visualizer
    void updateEQ();

    // Touch
    TouchButton checkTouch();

    // Notifications
    void showSeekingAnimation();
    void showMessage(const char* msg, uint16_t color = COLOR_TEXT);

private:
    TFT_eSPI _tft;
    TFT_eSprite _spriteFreq;
    TFT_eSprite _spriteEQ;
    RadioMode _currentMode;
    float _lastFrequency;
    uint8_t _lastSignal;
    bool _lastStereo;
    bool _lastMuted;
    uint8_t _lastVolume;
    unsigned long _lastTouchTime;
    unsigned long _lastEQUpdate;

    // EQ visualizer state
    uint8_t _eqBars[EQ_BARS];
    uint8_t _eqPeaks[EQ_BARS];
    uint8_t _eqPeakDelay[EQ_BARS];

    // Touch areas
    static const int MAX_BUTTONS = 12;
    TouchArea _buttons[MAX_BUTTONS];
    int _buttonCount;

    // Drawing helpers
    void drawHeader(RadioMode mode);
    void drawEQVisualizer();
    void drawVolumeSlider(uint8_t volume);
    void drawSignalMeter(uint8_t level);
    void drawPresetStrip(int index, float freq, int total);
    void drawGlowLine(int y, uint16_t color);

    // Touch buttons
    void drawPioneerButton(int16_t x, int16_t y, int16_t w, int16_t h,
                           const char* label, uint16_t glowColor, bool active = false);
    void drawFMControls(bool muted, const char* ledEffectName);
    void drawBTControls(bool muted, bool isPlaying, const char* ledEffectName);

    // Button management
    void registerButton(int16_t x, int16_t y, int16_t w, int16_t h, TouchButton id);
    void clearButtons();
    bool getTouchPoint(int16_t &x, int16_t &y);
    TouchButton hitTest(int16_t x, int16_t y);
};

#endif
