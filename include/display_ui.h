/**
 * ESP32 Radio Bluetooth - Display UI
 * ILI9341 TFT 240x320 User Interface
 */

#ifndef DISPLAY_UI_H
#define DISPLAY_UI_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include "config.h"
#include "TEA5767.h"

class DisplayUI {
public:
    DisplayUI();

    void begin();
    void drawSplashScreen();

    // Main screens
    void drawFMScreen(float frequency, bool stereo, uint8_t signalLevel,
                      bool muted, int presetIndex, int presetCount, uint8_t volume);
    void drawBluetoothScreen(bool connected, const char* deviceName,
                              bool muted, uint8_t volume, bool isPlaying);

    // Partial updates (avoid full redraw flicker)
    void updateFrequency(float frequency);
    void updateSignalLevel(uint8_t level);
    void updateStereoIndicator(bool stereo);
    void updateMuteIndicator(bool muted);
    void updateVolume(uint8_t volume);
    void updatePresetInfo(int presetIndex, float presetFreq, int presetCount);
    void updateBTStatus(bool connected, const char* deviceName);
    void updatePlayStatus(bool isPlaying);
    void updateModeIndicator(RadioMode mode);

    // Notifications
    void showSeekingAnimation();
    void showMessage(const char* msg, uint16_t color = COLOR_TEXT);

private:
    TFT_eSPI _tft;
    TFT_eSprite _spriteFreq;
    RadioMode _currentMode;
    float _lastFrequency;
    uint8_t _lastSignal;
    bool _lastStereo;
    bool _lastMuted;
    uint8_t _lastVolume;

    void drawHeader(RadioMode mode);
    void drawVolumeBar(uint8_t volume);
    void drawSignalBars(uint8_t level);
    void drawPresetBar(int index, float freq, int total);
    void drawDivider(int y);
};

#endif
