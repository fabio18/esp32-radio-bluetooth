/**
 * ESP32 Radio Bluetooth - Display UI
 * ILI9341 TFT 240x320 User Interface
 */

#include "display_ui.h"

DisplayUI::DisplayUI()
    : _tft(TFT_eSPI()),
      _spriteFreq(&_tft),
      _currentMode(MODE_FM),
      _lastFrequency(0),
      _lastSignal(0),
      _lastStereo(false),
      _lastMuted(false),
      _lastVolume(0)
{
}

void DisplayUI::begin() {
    _tft.init();
    _tft.setRotation(0); // Portrait 240x320
    _tft.fillScreen(COLOR_BG);
    _tft.setTextColor(COLOR_TEXT, COLOR_BG);

    _spriteFreq.createSprite(220, 50);
}

// ============================================
// Splash Screen
// ============================================
void DisplayUI::drawSplashScreen() {
    _tft.fillScreen(COLOR_BG);

    // Logo area
    _tft.fillRoundRect(60, 60, 120, 120, 20, COLOR_HEADER_BG);

    // Radio icon
    _tft.setTextDatum(MC_DATUM);
    _tft.setTextColor(COLOR_PRIMARY);
    _tft.drawString("((( )))", 120, 100, 4);

    // BT icon
    _tft.setTextColor(COLOR_BT_ACTIVE);
    _tft.drawString("BT", 120, 140, 4);

    // Title
    _tft.setTextColor(COLOR_TEXT);
    _tft.drawString("ESP32 Radio", 120, 210, 4);

    _tft.setTextColor(COLOR_TEXT_DIM);
    _tft.drawString("FM + Bluetooth", 120, 240, 2);
    _tft.drawString("v1.0", 120, 260, 2);

    // Loading bar
    for (int i = 0; i <= 100; i += 2) {
        int barWidth = (i * 160) / 100;
        _tft.fillRoundRect(40, 285, barWidth, 8, 4, COLOR_PRIMARY);
        delay(20);
    }

    delay(500);
}

// ============================================
// Header
// ============================================
void DisplayUI::drawHeader(RadioMode mode) {
    _tft.fillRect(0, 0, SCREEN_WIDTH, 36, COLOR_HEADER_BG);

    _tft.setTextDatum(ML_DATUM);
    _tft.setTextColor(COLOR_TEXT, COLOR_HEADER_BG);
    _tft.drawString("ESP32 Radio", 8, 18, 2);

    // Mode badge
    if (mode == MODE_FM) {
        _tft.fillRoundRect(170, 6, 60, 24, 6, COLOR_FM_ACTIVE);
        _tft.setTextDatum(MC_DATUM);
        _tft.setTextColor(COLOR_BG, COLOR_FM_ACTIVE);
        _tft.drawString("FM", 200, 18, 2);
    } else {
        _tft.fillRoundRect(160, 6, 72, 24, 6, COLOR_BT_ACTIVE);
        _tft.setTextDatum(MC_DATUM);
        _tft.setTextColor(COLOR_TEXT, COLOR_BT_ACTIVE);
        _tft.drawString("BT", 196, 18, 2);
    }

    drawDivider(36);
}

// ============================================
// FM Screen
// ============================================
void DisplayUI::drawFMScreen(float frequency, bool stereo, uint8_t signalLevel,
                              bool muted, int presetIndex, int presetCount,
                              uint8_t volume) {
    _tft.fillScreen(COLOR_BG);
    _currentMode = MODE_FM;

    drawHeader(MODE_FM);

    // Signal bars area (top right under header)
    drawSignalBars(signalLevel);

    // Stereo indicator
    updateStereoIndicator(stereo);

    // Frequency display (large, centered)
    updateFrequency(frequency);

    // "MHz" label
    _tft.setTextDatum(MC_DATUM);
    _tft.setTextColor(COLOR_TEXT_DIM, COLOR_BG);
    _tft.drawString("MHz", 120, 155, 2);

    drawDivider(170);

    // Preset bar
    float presetFreq = (presetIndex >= 0 && presetIndex < presetCount) ? frequency : 0;
    drawPresetBar(presetIndex, presetFreq, presetCount);

    drawDivider(210);

    // Mute indicator
    updateMuteIndicator(muted);

    // Volume bar
    drawVolumeBar(volume);

    // Bottom controls hint
    drawDivider(280);
    _tft.setTextDatum(MC_DATUM);
    _tft.setTextColor(COLOR_TEXT_DIM, COLOR_BG);
    _tft.drawString("SEEK", 40, 300, 1);
    _tft.drawString("PRESET", 120, 300, 1);
    _tft.drawString("MODE", 200, 300, 1);

    _lastFrequency = frequency;
    _lastSignal = signalLevel;
    _lastStereo = stereo;
    _lastMuted = muted;
    _lastVolume = volume;
}

// ============================================
// Bluetooth Screen
// ============================================
void DisplayUI::drawBluetoothScreen(bool connected, const char* deviceName,
                                     bool muted, uint8_t volume, bool isPlaying) {
    _tft.fillScreen(COLOR_BG);
    _currentMode = MODE_BLUETOOTH;

    drawHeader(MODE_BLUETOOTH);

    // Bluetooth icon area
    _tft.fillCircle(120, 90, 40, COLOR_HEADER_BG);

    // BT symbol
    _tft.setTextDatum(MC_DATUM);
    _tft.setTextColor(COLOR_BT_ACTIVE, COLOR_HEADER_BG);
    _tft.drawString("BT", 120, 90, 4);

    // Connection status
    updateBTStatus(connected, deviceName);

    drawDivider(180);

    // Play status
    updatePlayStatus(isPlaying);

    drawDivider(220);

    // Mute indicator
    updateMuteIndicator(muted);

    // Volume bar
    drawVolumeBar(volume);

    // Bottom hint
    drawDivider(280);
    _tft.setTextDatum(MC_DATUM);
    _tft.setTextColor(COLOR_TEXT_DIM, COLOR_BG);
    _tft.drawString("VOL-", 40, 300, 1);
    _tft.drawString("MUTE", 120, 300, 1);
    _tft.drawString("VOL+", 200, 300, 1);

    _lastMuted = muted;
    _lastVolume = volume;
}

// ============================================
// Partial Update Methods
// ============================================
void DisplayUI::updateFrequency(float frequency) {
    _spriteFreq.fillSprite(COLOR_BG);
    _spriteFreq.setTextDatum(MC_DATUM);
    _spriteFreq.setTextColor(COLOR_FREQ);

    char freqStr[10];
    snprintf(freqStr, sizeof(freqStr), "%.1f", frequency);
    _spriteFreq.drawString(freqStr, 110, 25, 7);

    _spriteFreq.pushSprite(10, 90);
    _lastFrequency = frequency;
}

void DisplayUI::updateSignalLevel(uint8_t level) {
    if (level == _lastSignal) return;
    drawSignalBars(level);
    _lastSignal = level;
}

void DisplayUI::updateStereoIndicator(bool stereo) {
    _tft.fillRect(8, 42, 80, 20, COLOR_BG);
    _tft.setTextDatum(ML_DATUM);
    if (stereo) {
        _tft.setTextColor(COLOR_SIGNAL, COLOR_BG);
        _tft.drawString("STEREO", 8, 52, 2);
    } else {
        _tft.setTextColor(COLOR_TEXT_DIM, COLOR_BG);
        _tft.drawString("MONO", 8, 52, 2);
    }
    _lastStereo = stereo;
}

void DisplayUI::updateMuteIndicator(bool muted) {
    _tft.fillRect(8, 225, 100, 22, COLOR_BG);
    _tft.setTextDatum(ML_DATUM);
    if (muted) {
        _tft.setTextColor(COLOR_MUTED, COLOR_BG);
        _tft.drawString("MUDO", 8, 236, 2);
    } else {
        _tft.setTextColor(COLOR_TEXT_DIM, COLOR_BG);
        _tft.drawString("Volume:", 8, 236, 2);
    }
    _lastMuted = muted;
}

void DisplayUI::updateVolume(uint8_t volume) {
    if (volume == _lastVolume) return;
    drawVolumeBar(volume);
    _lastVolume = volume;
}

void DisplayUI::updatePresetInfo(int presetIndex, float presetFreq, int presetCount) {
    drawPresetBar(presetIndex, presetFreq, presetCount);
}

void DisplayUI::updateBTStatus(bool connected, const char* deviceName) {
    _tft.fillRect(0, 140, SCREEN_WIDTH, 36, COLOR_BG);
    _tft.setTextDatum(MC_DATUM);

    if (connected) {
        _tft.setTextColor(COLOR_SIGNAL, COLOR_BG);
        _tft.drawString("Conectado", 120, 148, 2);
        _tft.setTextColor(COLOR_TEXT, COLOR_BG);
        _tft.drawString(deviceName ? deviceName : "Alexa", 120, 168, 2);
    } else {
        _tft.setTextColor(COLOR_SECONDARY, COLOR_BG);
        _tft.drawString("Aguardando...", 120, 148, 2);
        _tft.setTextColor(COLOR_TEXT_DIM, COLOR_BG);
        _tft.drawString(BT_DEVICE_NAME, 120, 168, 1);
    }
}

void DisplayUI::updatePlayStatus(bool isPlaying) {
    _tft.fillRect(60, 190, 120, 25, COLOR_BG);
    _tft.setTextDatum(MC_DATUM);
    if (isPlaying) {
        _tft.setTextColor(COLOR_SIGNAL, COLOR_BG);
        _tft.drawString(">> Tocando >>", 120, 202, 2);
    } else {
        _tft.setTextColor(COLOR_TEXT_DIM, COLOR_BG);
        _tft.drawString("|| Pausado ||", 120, 202, 2);
    }
}

void DisplayUI::updateModeIndicator(RadioMode mode) {
    if (mode == _currentMode) return;
    _currentMode = mode;
    drawHeader(mode);
}

void DisplayUI::showSeekingAnimation() {
    _tft.fillRect(10, 90, 220, 50, COLOR_BG);
    _tft.setTextDatum(MC_DATUM);
    _tft.setTextColor(COLOR_SECONDARY, COLOR_BG);
    _tft.drawString("Buscando...", 120, 115, 4);
}

void DisplayUI::showMessage(const char* msg, uint16_t color) {
    _tft.fillRect(0, 285, SCREEN_WIDTH, 35, COLOR_BG);
    _tft.setTextDatum(MC_DATUM);
    _tft.setTextColor(color, COLOR_BG);
    _tft.drawString(msg, 120, 300, 2);
}

// ============================================
// Private Drawing Helpers
// ============================================
void DisplayUI::drawVolumeBar(uint8_t volume) {
    int barX = 8;
    int barY = 252;
    int barW = SCREEN_WIDTH - 48;
    int barH = 16;

    // Background
    _tft.fillRoundRect(barX, barY, barW, barH, 4, COLOR_VOLUME_BG);

    // Fill
    int fillW = (volume * barW) / VOLUME_MAX;
    uint16_t fillColor = COLOR_VOLUME_BAR;
    if (volume > 80) fillColor = COLOR_SECONDARY;
    if (volume > 95) fillColor = COLOR_MUTED;

    if (fillW > 0) {
        _tft.fillRoundRect(barX, barY, fillW, barH, 4, fillColor);
    }

    // Percentage text
    char volStr[6];
    snprintf(volStr, sizeof(volStr), "%d%%", volume);
    _tft.setTextDatum(ML_DATUM);
    _tft.setTextColor(COLOR_TEXT, COLOR_BG);
    _tft.fillRect(barX + barW + 4, barY, 36, barH, COLOR_BG);
    _tft.drawString(volStr, barX + barW + 4, barY + 8, 2);
}

void DisplayUI::drawSignalBars(uint8_t level) {
    int startX = 180;
    int startY = 42;
    int barW = 8;
    int barGap = 4;

    // Clear area
    _tft.fillRect(startX, startY, 60, 25, COLOR_BG);

    for (int i = 0; i < 5; i++) {
        int barH = 6 + (i * 4);
        int x = startX + i * (barW + barGap);
        int y = startY + 24 - barH;

        uint16_t color;
        if (i < (int)(level * 5 / 15)) {
            if (level >= 10) color = COLOR_SIGNAL;
            else if (level >= 5) color = COLOR_SIGNAL_LOW;
            else color = COLOR_SIGNAL_NONE;
        } else {
            color = COLOR_VOLUME_BG;
        }
        _tft.fillRect(x, y, barW, barH, color);
    }
}

void DisplayUI::drawPresetBar(int index, float freq, int total) {
    _tft.fillRect(0, 175, SCREEN_WIDTH, 32, COLOR_BG);

    if (total <= 0 || index < 0) {
        _tft.setTextDatum(MC_DATUM);
        _tft.setTextColor(COLOR_TEXT_DIM, COLOR_BG);
        _tft.drawString("Sem preset", 120, 191, 2);
        return;
    }

    // Preset label
    _tft.setTextDatum(ML_DATUM);
    _tft.setTextColor(COLOR_PRIMARY, COLOR_BG);

    char presetStr[32];
    snprintf(presetStr, sizeof(presetStr), "Preset %d/%d", index + 1, total);
    _tft.drawString(presetStr, 8, 185, 2);

    // Frequency
    char freqStr[16];
    snprintf(freqStr, sizeof(freqStr), "%.1f MHz", freq);
    _tft.setTextDatum(MR_DATUM);
    _tft.setTextColor(COLOR_TEXT, COLOR_BG);
    _tft.drawString(freqStr, 232, 185, 2);

    // Dots for preset position
    int dotY = 200;
    int totalDots = min(total, 10);
    int dotSpacing = SCREEN_WIDTH / (totalDots + 1);
    for (int i = 0; i < totalDots; i++) {
        int dotX = dotSpacing * (i + 1);
        if (i == index % 10) {
            _tft.fillCircle(dotX, dotY, 4, COLOR_PRIMARY);
        } else {
            _tft.fillCircle(dotX, dotY, 2, COLOR_TEXT_DIM);
        }
    }
}

void DisplayUI::drawDivider(int y) {
    _tft.drawFastHLine(8, y, SCREEN_WIDTH - 16, COLOR_DIVIDER);
}
