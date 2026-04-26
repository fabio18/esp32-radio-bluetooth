/**
 * ESP32 Radio Bluetooth - Display UI Pioneer Style
 * ILI9341 TFT 240x320 + XPT2046 Touch
 * Visual automotivo estilo Pioneer
 *
 * Layout (240x320 portrait):
 *  [0-32]   Header: logo + source badge
 *  [33-56]  Status: stereo/mono + signal meter
 *  [57-140] Frequency display (large cyan digits)
 *  [141-175] EQ Visualizer animated bars
 *  [176-194] Preset strip
 *  [195-232] Row 1: Seek/Preset buttons
 *  [233-268] Row 2: Vol-/Mute/Vol+
 *  [269-310] Row 3: Mode switch button
 *  [311-319] Volume slider thin bar
 */

#include "display_ui.h"

DisplayUI::DisplayUI()
    : _tft(TFT_eSPI()),
      _spriteFreq(&_tft),
      _spriteEQ(&_tft),
      _currentMode(MODE_FM),
      _lastFrequency(0),
      _lastSignal(0),
      _lastStereo(false),
      _lastMuted(false),
      _lastVolume(0),
      _lastTouchTime(0),
      _lastEQUpdate(0),
      _buttonCount(0)
{
    memset(_eqBars, 0, sizeof(_eqBars));
    memset(_eqPeaks, 0, sizeof(_eqPeaks));
    memset(_eqPeakDelay, 0, sizeof(_eqPeakDelay));
}

void DisplayUI::begin() {
    _tft.init();
    _tft.setRotation(0);
    _tft.fillScreen(COLOR_BG);

    _spriteFreq.createSprite(224, 55);
    _spriteEQ.createSprite(224, EQ_MAX_HEIGHT + 4);

    uint16_t calData[5] = {TOUCH_MIN_X, TOUCH_MAX_X, TOUCH_MIN_Y, TOUCH_MAX_Y, 1};
    _tft.setTouch(calData);
}

void DisplayUI::calibrateTouch() {
    _tft.fillScreen(COLOR_BG);
    _tft.setTextDatum(MC_DATUM);
    _tft.setTextColor(COLOR_NEON_CYAN, COLOR_BG);
    _tft.drawString("CALIBRACAO", 120, 140, 4);
    _tft.setTextColor(COLOR_TEXT_DIM, COLOR_BG);
    _tft.drawString("Toque nos pontos", 120, 170, 2);

    uint16_t calData[5];
    _tft.calibrateTouch(calData, COLOR_NEON_CYAN, COLOR_BG, 15);

    Serial.printf("[TOUCH] Cal: %d,%d,%d,%d,%d\n",
                  calData[0], calData[1], calData[2], calData[3], calData[4]);
    delay(500);
}

// ============================================
// Pioneer Splash Screen
// ============================================
void DisplayUI::drawSplashScreen() {
    _tft.fillScreen(COLOR_BG);

    // Glow lines top
    for (int i = 0; i < 3; i++) {
        _tft.drawFastHLine(20 + i * 5, 40 + i * 2, 200 - i * 10, COLOR_FREQ_GLOW);
    }

    // Main logo text with "glow" effect
    _tft.setTextDatum(MC_DATUM);

    // Glow layer (slightly offset, dimmer)
    _tft.setTextColor(COLOR_FREQ_GLOW);
    _tft.drawString("ESP32", 121, 101, 4);
    _tft.drawString("RADIO", 121, 141, 4);

    // Main layer
    _tft.setTextColor(COLOR_NEON_CYAN);
    _tft.drawString("ESP32", 120, 100, 4);
    _tft.drawString("RADIO", 120, 140, 4);

    // Subtitle
    _tft.setTextColor(COLOR_NEON_BLUE);
    _tft.drawString("FM + BLUETOOTH", 120, 180, 2);

    // Pioneer-style thin line
    drawGlowLine(200, COLOR_NEON_CYAN);

    _tft.setTextColor(COLOR_TEXT_DIM);
    _tft.drawString("TOUCH CONTROL", 120, 220, 2);
    _tft.drawString("v2.0", 120, 245, 1);

    // Loading bar with glow
    int barY = 275;
    _tft.drawRoundRect(29, barY - 1, 182, 12, 4, COLOR_FREQ_GLOW);
    for (int i = 0; i <= 100; i += 2) {
        int w = (i * 178) / 100;
        // Gradient fill: green -> cyan
        uint16_t color = (i < 50) ? COLOR_NEON_GREEN : COLOR_NEON_CYAN;
        _tft.fillRoundRect(31, barY + 1, w, 8, 3, color);
        delay(15);
    }
    delay(400);
}

// ============================================
// Glow Line (Pioneer-style divider)
// ============================================
void DisplayUI::drawGlowLine(int y, uint16_t color) {
    _tft.drawFastHLine(8, y - 1, SCREEN_WIDTH - 16, COLOR_FREQ_GLOW);
    _tft.drawFastHLine(4, y, SCREEN_WIDTH - 8, color);
    _tft.drawFastHLine(8, y + 1, SCREEN_WIDTH - 16, COLOR_FREQ_GLOW);
}

// ============================================
// Header Bar
// ============================================
void DisplayUI::drawHeader(RadioMode mode) {
    _tft.fillRect(0, 0, SCREEN_WIDTH, 33, COLOR_HEADER_BG);

    // Subtle top highlight line
    _tft.drawFastHLine(0, 0, SCREEN_WIDTH, COLOR_BTN_BORDER);

    // "ESP32" logo text
    _tft.setTextDatum(ML_DATUM);
    _tft.setTextColor(COLOR_NEON_CYAN, COLOR_HEADER_BG);
    _tft.drawString("ESP32", 6, 16, 2);
    _tft.setTextColor(COLOR_TEXT_DIM, COLOR_HEADER_BG);
    _tft.drawString("RADIO", 52, 16, 2);

    // Source badge (touchable mode indicator)
    if (mode == MODE_FM) {
        _tft.fillRoundRect(176, 5, 56, 22, 4, COLOR_BTN_MODE_FM);
        _tft.drawRoundRect(176, 5, 56, 22, 4, COLOR_NEON_GREEN);
        _tft.setTextDatum(MC_DATUM);
        _tft.setTextColor(COLOR_BG, COLOR_BTN_MODE_FM);
        _tft.drawString("FM", 204, 16, 2);
    } else {
        _tft.fillRoundRect(170, 5, 64, 22, 4, COLOR_BTN_MODE_BT);
        _tft.drawRoundRect(170, 5, 64, 22, 4, COLOR_NEON_BLUE);
        _tft.setTextDatum(MC_DATUM);
        _tft.setTextColor(COLOR_NEON_WHITE, COLOR_BTN_MODE_BT);
        _tft.drawString("BT", 202, 16, 2);
    }

    // Bottom glow line
    _tft.drawFastHLine(0, 32, SCREEN_WIDTH, COLOR_BTN_BORDER);
}

// ============================================
// Signal Meter (Pioneer-style horizontal bars)
// ============================================
void DisplayUI::drawSignalMeter(uint8_t level) {
    int x = 160;
    int y = 40;

    _tft.fillRect(x, y, 76, 14, COLOR_BG);

    // "SIG" label
    _tft.setTextDatum(ML_DATUM);
    _tft.setTextColor(COLOR_TEXT_DIM, COLOR_BG);
    _tft.drawString("SIG", x, y + 7, 1);

    // 5 bars
    int barX = x + 22;
    for (int i = 0; i < 5; i++) {
        uint16_t color;
        if (i < (int)(level * 5 / 15)) {
            if (i < 2) color = COLOR_NEON_GREEN;
            else if (i < 4) color = COLOR_NEON_CYAN;
            else color = COLOR_NEON_BLUE;
        } else {
            color = COLOR_EQ_BG;
        }
        _tft.fillRect(barX + i * 11, y + 2, 9, 10, color);
    }
}

// ============================================
// EQ Visualizer (animated spectrum bars)
// ============================================
void DisplayUI::drawEQVisualizer() {
    _spriteEQ.fillSprite(COLOR_BG);

    int barW = 12;
    int gap = 2;
    int startX = (224 - EQ_BARS * (barW + gap)) / 2;

    for (int i = 0; i < EQ_BARS; i++) {
        int x = startX + i * (barW + gap);
        int h = _eqBars[i];
        int y = EQ_MAX_HEIGHT - h;

        // Gradient bar: green at bottom -> cyan -> blue at top
        for (int row = 0; row < h; row++) {
            int drawY = y + row;
            float ratio = (float)(h - row) / EQ_MAX_HEIGHT;
            uint16_t color;
            if (ratio < 0.33) color = COLOR_EQ_LOW;
            else if (ratio < 0.66) color = COLOR_EQ_MID;
            else color = COLOR_EQ_HIGH;
            _spriteEQ.drawFastHLine(x, drawY, barW, color);
        }

        // Peak indicator
        if (_eqPeaks[i] > 0) {
            int peakY = EQ_MAX_HEIGHT - _eqPeaks[i];
            _spriteEQ.drawFastHLine(x, peakY, barW, COLOR_EQ_PEAK);
        }
    }

    _spriteEQ.pushSprite(8, 142);
}

void DisplayUI::updateEQ() {
    unsigned long now = millis();
    if (now - _lastEQUpdate < EQ_UPDATE_MS) return;
    _lastEQUpdate = now;

    for (int i = 0; i < EQ_BARS; i++) {
        // Simulate EQ movement (random-ish, biased by position)
        int target = random(3, EQ_MAX_HEIGHT - 2);
        // Smooth approach
        if (_eqBars[i] < target) _eqBars[i] += random(1, 4);
        else if (_eqBars[i] > target) _eqBars[i] -= random(1, 3);
        if (_eqBars[i] > EQ_MAX_HEIGHT) _eqBars[i] = EQ_MAX_HEIGHT;

        // Peak tracking
        if (_eqBars[i] >= _eqPeaks[i]) {
            _eqPeaks[i] = _eqBars[i];
            _eqPeakDelay[i] = 6;
        } else {
            if (_eqPeakDelay[i] > 0) {
                _eqPeakDelay[i]--;
            } else {
                if (_eqPeaks[i] > 0) _eqPeaks[i]--;
            }
        }
    }

    drawEQVisualizer();
}

// ============================================
// Pioneer-style Touch Button
// ============================================
void DisplayUI::drawPioneerButton(int16_t x, int16_t y, int16_t w, int16_t h,
                                   const char* label, uint16_t glowColor, bool active) {
    // Dark fill
    uint16_t bg = active ? COLOR_BTN_PRESSED : COLOR_BTN_BG;
    _tft.fillRoundRect(x + 1, y + 1, w - 2, h - 2, 5, bg);

    // Glow border
    _tft.drawRoundRect(x, y, w, h, 5, glowColor);

    // Subtle inner highlight (top edge)
    _tft.drawFastHLine(x + 4, y + 1, w - 8, COLOR_DIVIDER);

    // Label
    _tft.setTextDatum(MC_DATUM);
    _tft.setTextColor(glowColor, bg);
    _tft.drawString(label, x + w / 2, y + h / 2, 2);
}

// ============================================
// Button Management
// ============================================
void DisplayUI::clearButtons() {
    _buttonCount = 0;
}

void DisplayUI::registerButton(int16_t x, int16_t y, int16_t w, int16_t h, TouchButton id) {
    if (_buttonCount >= MAX_BUTTONS) return;
    _buttons[_buttonCount] = {x, y, w, h, id};
    _buttonCount++;
}

bool DisplayUI::getTouchPoint(int16_t &x, int16_t &y) {
    uint16_t tx, ty;
    if (_tft.getTouch(&tx, &ty)) {
        x = (int16_t)tx;
        y = (int16_t)ty;
        return true;
    }
    return false;
}

TouchButton DisplayUI::hitTest(int16_t x, int16_t y) {
    for (int i = 0; i < _buttonCount; i++) {
        TouchArea &b = _buttons[i];
        if (x >= b.x && x <= b.x + b.w && y >= b.y && y <= b.y + b.h) {
            return b.id;
        }
    }
    return BTN_NONE;
}

TouchButton DisplayUI::checkTouch() {
    unsigned long now = millis();
    if (now - _lastTouchTime < TOUCH_DEBOUNCE_MS) return BTN_NONE;

    int16_t x, y;
    if (!getTouchPoint(x, y)) return BTN_NONE;

    TouchButton btn = hitTest(x, y);
    if (btn != BTN_NONE) {
        _lastTouchTime = now;
    }
    return btn;
}

// ============================================
// FM Screen
// ============================================
void DisplayUI::drawFMScreen(float frequency, bool stereo, uint8_t signalLevel,
                              bool muted, int presetIndex, int presetCount,
                              uint8_t volume, const char* ledEffectName) {
    _tft.fillScreen(COLOR_BG);
    _currentMode = MODE_FM;
    clearButtons();

    drawHeader(MODE_FM);

    // Status line
    updateStereoIndicator(stereo);
    drawSignalMeter(signalLevel);

    // Pioneer glow line under status
    _tft.drawFastHLine(4, 56, SCREEN_WIDTH - 8, COLOR_DIVIDER);

    // Frequency display
    updateFrequency(frequency);

    // "FM MHz" label
    _tft.setTextDatum(MC_DATUM);
    _tft.setTextColor(COLOR_TEXT_DIM, COLOR_BG);
    _tft.drawString("FM MHz", 120, 130, 2);

    // EQ Visualizer
    for (int i = 0; i < EQ_BARS; i++) {
        _eqBars[i] = random(5, EQ_MAX_HEIGHT);
        _eqPeaks[i] = _eqBars[i];
        _eqPeakDelay[i] = 3;
    }
    drawEQVisualizer();

    // Glow line
    _tft.drawFastHLine(4, 176, SCREEN_WIDTH - 8, COLOR_DIVIDER);

    // Preset strip
    float presetFreq = (presetIndex >= 0 && presetIndex < presetCount) ? frequency : 0;
    drawPresetStrip(presetIndex, presetFreq, presetCount);

    // Volume slider
    drawVolumeSlider(volume);

    // Touch buttons
    drawFMControls(muted, ledEffectName);

    _lastFrequency = frequency;
    _lastSignal = signalLevel;
    _lastStereo = stereo;
    _lastMuted = muted;
    _lastVolume = volume;
}

// ============================================
// FM Touch Controls
// ============================================
void DisplayUI::drawFMControls(bool muted, const char* ledEffectName) {
    int row1Y = 197;
    int row2Y = 233;
    int row3Y = 265;
    int row4Y = 293;
    int btnH = 30;

    // Row 1: [<< SEEK] [< P] [P >] [SEEK >>]
    drawPioneerButton(4, row1Y, 68, btnH, "<< SEEK", COLOR_NEON_CYAN);
    registerButton(4, row1Y, 68, btnH, BTN_SEEK_DOWN);

    drawPioneerButton(76, row1Y, 42, btnH, "<P", COLOR_NEON_ORANGE);
    registerButton(76, row1Y, 42, btnH, BTN_PRESET_PREV);

    drawPioneerButton(122, row1Y, 42, btnH, "P>", COLOR_NEON_ORANGE);
    registerButton(122, row1Y, 42, btnH, BTN_PRESET_NEXT);

    drawPioneerButton(168, row1Y, 68, btnH, "SEEK >>", COLOR_NEON_CYAN);
    registerButton(168, row1Y, 68, btnH, BTN_SEEK_UP);

    // Row 2: [VOL -] [MUTE] [VOL +]
    drawPioneerButton(4, row2Y, 66, btnH, "VOL -", COLOR_NEON_GREEN);
    registerButton(4, row2Y, 66, btnH, BTN_VOL_DOWN);

    if (muted) {
        drawPioneerButton(74, row2Y, 92, btnH, "MUDO", COLOR_NEON_RED, true);
    } else {
        drawPioneerButton(74, row2Y, 92, btnH, "MUTE", COLOR_NEON_RED);
    }
    registerButton(74, row2Y, 92, btnH, BTN_MUTE);

    drawPioneerButton(170, row2Y, 66, btnH, "VOL +", COLOR_NEON_GREEN);
    registerButton(170, row2Y, 66, btnH, BTN_VOL_UP);

    // Row 3: [LED MODE] [LED BRIGHT]
    drawPioneerButton(4, row3Y, 136, btnH - 4, ledEffectName, COLOR_NEON_MAGENTA);
    registerButton(4, row3Y, 136, btnH - 4, BTN_LED_MODE);

    drawPioneerButton(144, row3Y, 92, btnH - 4, "BRILHO", COLOR_NEON_MAGENTA);
    registerButton(144, row3Y, 92, btnH - 4, BTN_LED_BRIGHT);

    // Row 4: [BLUETOOTH mode switch]
    drawPioneerButton(4, row4Y, SCREEN_WIDTH - 8, btnH - 4, "BLUETOOTH", COLOR_NEON_BLUE);
    registerButton(4, row4Y, SCREEN_WIDTH - 8, btnH - 4, BTN_MODE);
}

// ============================================
// Bluetooth Screen
// ============================================
void DisplayUI::drawBluetoothScreen(bool connected, const char* deviceName,
                                     bool muted, uint8_t volume, bool isPlaying,
                                     const char* ledEffectName) {
    _tft.fillScreen(COLOR_BG);
    _currentMode = MODE_BLUETOOTH;
    clearButtons();

    drawHeader(MODE_BLUETOOTH);

    // BT icon with glow effect
    _tft.fillCircle(120, 78, 30, COLOR_HEADER_BG);
    _tft.drawCircle(120, 78, 30, COLOR_BTN_BORDER);
    _tft.drawCircle(120, 78, 32, COLOR_FREQ_GLOW);
    _tft.setTextDatum(MC_DATUM);
    _tft.setTextColor(COLOR_BT_ICON, COLOR_HEADER_BG);
    _tft.drawString("BT", 120, 78, 4);

    // Status
    updateBTStatus(connected, deviceName);
    updatePlayStatus(isPlaying);

    // EQ Visualizer
    for (int i = 0; i < EQ_BARS; i++) {
        _eqBars[i] = isPlaying ? random(5, EQ_MAX_HEIGHT) : random(1, 5);
        _eqPeaks[i] = _eqBars[i];
        _eqPeakDelay[i] = 3;
    }
    drawEQVisualizer();

    _tft.drawFastHLine(4, 176, SCREEN_WIDTH - 8, COLOR_DIVIDER);

    // Volume
    drawVolumeSlider(volume);

    // Touch controls
    drawBTControls(muted, isPlaying, ledEffectName);

    _lastMuted = muted;
    _lastVolume = volume;
}

// ============================================
// BT Touch Controls
// ============================================
void DisplayUI::drawBTControls(bool muted, bool isPlaying, const char* ledEffectName) {
    int row1Y = 197;
    int row2Y = 231;
    int row3Y = 261;
    int row4Y = 293;
    int btnH = 30;

    // Row 1: [PLAY/PAUSE]
    if (isPlaying) {
        drawPioneerButton(4, row1Y, SCREEN_WIDTH - 8, btnH, "|| PAUSAR", COLOR_NEON_ORANGE, true);
    } else {
        drawPioneerButton(4, row1Y, SCREEN_WIDTH - 8, btnH, "> TOCAR", COLOR_NEON_GREEN);
    }
    registerButton(4, row1Y, SCREEN_WIDTH - 8, btnH, BTN_PLAY_PAUSE);

    // Row 2: [VOL -] [MUTE] [VOL +]
    drawPioneerButton(4, row2Y, 66, btnH, "VOL -", COLOR_NEON_GREEN);
    registerButton(4, row2Y, 66, btnH, BTN_VOL_DOWN);

    if (muted) {
        drawPioneerButton(74, row2Y, 92, btnH, "MUDO", COLOR_NEON_RED, true);
    } else {
        drawPioneerButton(74, row2Y, 92, btnH, "MUTE", COLOR_NEON_RED);
    }
    registerButton(74, row2Y, 92, btnH, BTN_MUTE);

    drawPioneerButton(170, row2Y, 66, btnH, "VOL +", COLOR_NEON_GREEN);
    registerButton(170, row2Y, 66, btnH, BTN_VOL_UP);

    // Row 3: [LED MODE] [LED BRIGHT]
    drawPioneerButton(4, row3Y, 136, btnH - 4, ledEffectName, COLOR_NEON_MAGENTA);
    registerButton(4, row3Y, 136, btnH - 4, BTN_LED_MODE);

    drawPioneerButton(144, row3Y, 92, btnH - 4, "BRILHO", COLOR_NEON_MAGENTA);
    registerButton(144, row3Y, 92, btnH - 4, BTN_LED_BRIGHT);

    // Row 4: [FM RADIO mode switch]
    drawPioneerButton(4, row4Y, SCREEN_WIDTH - 8, btnH - 4, "FM RADIO", COLOR_NEON_GREEN);
    registerButton(4, row4Y, SCREEN_WIDTH - 8, btnH - 4, BTN_MODE);
}

// ============================================
// Partial Updates
// ============================================
void DisplayUI::updateFrequency(float frequency) {
    _spriteFreq.fillSprite(COLOR_BG);
    _spriteFreq.setTextDatum(MC_DATUM);

    char freqStr[10];
    snprintf(freqStr, sizeof(freqStr), "%.1f", frequency);

    // Glow shadow
    _spriteFreq.setTextColor(COLOR_FREQ_GLOW);
    _spriteFreq.drawString(freqStr, 113, 28, 7);

    // Main text
    _spriteFreq.setTextColor(COLOR_FREQ);
    _spriteFreq.drawString(freqStr, 112, 27, 7);

    _spriteFreq.pushSprite(8, 65);
    _lastFrequency = frequency;
}

void DisplayUI::updateSignalLevel(uint8_t level) {
    if (level == _lastSignal) return;
    drawSignalMeter(level);
    _lastSignal = level;
}

void DisplayUI::updateStereoIndicator(bool stereo) {
    _tft.fillRect(4, 38, 100, 16, COLOR_BG);
    _tft.setTextDatum(ML_DATUM);
    if (stereo) {
        _tft.setTextColor(COLOR_STEREO, COLOR_BG);
        _tft.drawString("STEREO", 6, 46, 2);
    } else {
        _tft.setTextColor(COLOR_TEXT_DIM, COLOR_BG);
        _tft.drawString("MONO", 6, 46, 2);
    }
    _lastStereo = stereo;
}

void DisplayUI::updateMuteIndicator(bool muted) {
    _lastMuted = muted;
    // Redraw mute button at current row2Y position
    int row2Y = (_currentMode == MODE_FM) ? 233 : 231;
    if (muted) {
        drawPioneerButton(74, row2Y, 92, 30, "MUDO", COLOR_NEON_RED, true);
    } else {
        drawPioneerButton(74, row2Y, 92, 30, "MUTE", COLOR_NEON_RED);
    }
}

void DisplayUI::updateVolume(uint8_t volume) {
    if (volume == _lastVolume) return;
    drawVolumeSlider(volume);
    _lastVolume = volume;
}

void DisplayUI::updatePresetInfo(int presetIndex, float presetFreq, int presetCount) {
    drawPresetStrip(presetIndex, presetFreq, presetCount);
}

void DisplayUI::updateBTStatus(bool connected, const char* deviceName) {
    _tft.fillRect(0, 112, SCREEN_WIDTH, 28, COLOR_BG);
    _tft.setTextDatum(MC_DATUM);

    if (connected) {
        _tft.setTextColor(COLOR_BT_CONNECTED, COLOR_BG);
        _tft.drawString("CONECTADO", 120, 118, 2);
        _tft.setTextColor(COLOR_TEXT, COLOR_BG);
        _tft.drawString(deviceName ? deviceName : "Alexa", 120, 134, 2);
    } else {
        _tft.setTextColor(COLOR_BT_WAITING, COLOR_BG);
        _tft.drawString("AGUARDANDO...", 120, 118, 2);
        _tft.setTextColor(COLOR_TEXT_DIM, COLOR_BG);
        _tft.drawString(BT_DEVICE_NAME, 120, 134, 1);
    }
}

void DisplayUI::updatePlayStatus(bool isPlaying) {
    _tft.fillRect(40, 136, 160, 16, COLOR_BG);
    _tft.setTextDatum(MC_DATUM);
    if (isPlaying) {
        _tft.setTextColor(COLOR_NEON_GREEN, COLOR_BG);
        _tft.drawString(">> TOCANDO >>", 120, 137, 1);
    } else {
        _tft.setTextColor(COLOR_TEXT_DIM, COLOR_BG);
        _tft.drawString("PAUSADO", 120, 137, 1);
    }
}

void DisplayUI::showSeekingAnimation() {
    _spriteFreq.fillSprite(COLOR_BG);
    _spriteFreq.setTextDatum(MC_DATUM);
    _spriteFreq.setTextColor(COLOR_NEON_ORANGE);
    _spriteFreq.drawString("SEEK...", 112, 27, 4);
    _spriteFreq.pushSprite(8, 65);
}

void DisplayUI::showMessage(const char* msg, uint16_t color) {
    _tft.fillRect(0, 178, SCREEN_WIDTH, 16, COLOR_BG);
    _tft.setTextDatum(MC_DATUM);
    _tft.setTextColor(color, COLOR_BG);
    _tft.drawString(msg, 120, 186, 2);
}

// ============================================
// Volume Slider (Pioneer-style thin bar at bottom)
// ============================================
void DisplayUI::drawVolumeSlider(uint8_t volume) {
    int barX = 4;
    int barY = 313;
    int barW = SCREEN_WIDTH - 40;
    int barH = 6;

    // Track background
    _tft.fillRoundRect(barX, barY, barW, barH, 3, COLOR_VOL_BG);

    // Fill
    int fillW = (volume * barW) / VOLUME_MAX;
    uint16_t fillColor = COLOR_VOL_BAR;
    if (volume > 80) fillColor = COLOR_VOL_HIGH;
    if (volume > 95) fillColor = COLOR_VOL_MAX;

    if (fillW > 0) {
        _tft.fillRoundRect(barX, barY, fillW, barH, 3, fillColor);
        // Glow dot at end
        _tft.fillCircle(barX + fillW, barY + 3, 4, fillColor);
    }

    // Volume text
    char volStr[6];
    snprintf(volStr, sizeof(volStr), "%d", volume);
    _tft.fillRect(barX + barW + 2, barY - 4, 36, 14, COLOR_BG);
    _tft.setTextDatum(ML_DATUM);
    _tft.setTextColor(fillColor, COLOR_BG);
    _tft.drawString(volStr, barX + barW + 6, barY + 2, 2);
}

// ============================================
// Preset Strip
// ============================================
void DisplayUI::drawPresetStrip(int index, float freq, int total) {
    _tft.fillRect(0, 178, SCREEN_WIDTH, 16, COLOR_BG);

    if (total <= 0 || index < 0) {
        _tft.setTextDatum(MC_DATUM);
        _tft.setTextColor(COLOR_TEXT_DIM, COLOR_BG);
        _tft.drawString("SEM PRESET", 120, 186, 1);
        return;
    }

    // "P3/10  93.7 MHz" style
    char str[32];
    snprintf(str, sizeof(str), "P%d/%d  %.1f MHz", index + 1, total, freq);
    _tft.setTextDatum(MC_DATUM);
    _tft.setTextColor(COLOR_NEON_ORANGE, COLOR_BG);
    _tft.drawString(str, 120, 186, 2);
}

// ============================================
// Signal Meter
// ============================================
// (drawSignalMeter is defined above in the Signal Meter section)
