/**
 * ESP32 Kindlelaser Max 260W - Display UI Implementation
 * Interface touch industrial para controle de corte de metal
 *
 * Layout da tela (240x320):
 * ┌─────────────────────────────┐
 * │ KINDLELASER 260W    [STATUS]│  Header (0-24)
 * ├─────────────────────────────┤
 * │ Material: Aco 2.0mm  [O2]  │  Preset (25-49)
 * ├─────────────────────────────┤
 * │   POTENCIA: 95%  (247W)    │  Power display (50-89)
 * │   VELOCIDADE: 12 mm/s      │  Speed display (90-119)
 * ├─────────────────────────────┤
 * │ [POT-] [POT+] [SPD-] [SPD+]│  Controls row 1 (120-155)
 * │ [<MAT]  [PULSE]  [MAT>]   │  Controls row 2 (156-191)
 * │ [AR]  [EXAUST]  [E-STOP]  │  Controls row 3 (192-227)
 * │ [═══ LASER ON/OFF ════════]│  Laser button (228-263)
 * ├─────────────────────────────┤
 * │ ████████████░░░░░  95%     │  Power bar (264-283)
 * ├─────────────────────────────┤
 * │ 💧OK  🚪OK  🌡25°C  ⏱02:30│  Safety bar (284-319)
 * └─────────────────────────────┘
 */

#include "display_ui.h"

// ============================================
// Constructor
// ============================================
DisplayUI::DisplayUI()
    : _tft(),
      _spritePower(&_tft),
      _spriteStatus(&_tft),
      _lastTouchTime(0),
      _lastPower(0),
      _lastSpeed(0),
      _lastLaserOn(false),
      _lastMode(MODE_IDLE),
      _buttonCount(0)
{
}

// ============================================
// Initialization
// ============================================
void DisplayUI::begin() {
    _tft.init();
    _tft.setRotation(0); // Portrait 240x320
    _tft.fillScreen(COLOR_BG);
    _tft.setTextColor(COLOR_TEXT, COLOR_BG);

    // Initialize sprites
    _spritePower.createSprite(220, 35);
    _spriteStatus.createSprite(240, 20);

    Serial.println("[DISPLAY] ILI9341 + XPT2046 initialized (240x320)");
}

void DisplayUI::calibrateTouch() {
    _tft.fillScreen(COLOR_BG);
    _tft.setTextColor(COLOR_NEON_CYAN, COLOR_BG);
    _tft.drawString("CALIBRACAO TOUCH", 40, 150, 2);
    _tft.drawString("Toque nos 4 cantos", 40, 170, 2);

    uint16_t calData[5];
    _tft.calibrateTouch(calData, COLOR_NEON_CYAN, COLOR_BG, 15);

    Serial.println("[TOUCH] Calibration data:");
    for (int i = 0; i < 5; i++) {
        Serial.printf("  calData[%d] = %d\n", i, calData[i]);
    }
}

// ============================================
// Splash Screen
// ============================================
void DisplayUI::drawSplashScreen() {
    _tft.fillScreen(COLOR_BG);

    // Logo text
    _tft.setTextColor(COLOR_NEON_RED, COLOR_BG);
    _tft.setTextDatum(TC_DATUM);
    _tft.drawString("KINDLELASER", 120, 80, 4);

    _tft.setTextColor(COLOR_NEON_CYAN, COLOR_BG);
    _tft.drawString("MAX 260W", 120, 120, 4);

    // Subtitle
    _tft.setTextColor(COLOR_TEXT, COLOR_BG);
    _tft.drawString("Metal Cutting Controller", 120, 165, 2);

    // Glow line
    drawGlowLine(155, COLOR_NEON_RED);

    // Version
    _tft.setTextColor(COLOR_TEXT_DIM, COLOR_BG);
    _tft.drawString("ESP32 Touch Panel v1.0", 120, 200, 1);
    _tft.drawString("CO2 Laser - Corte de Metal", 120, 215, 1);

    // Safety reminder
    _tft.setTextColor(COLOR_NEON_ORANGE, COLOR_BG);
    _tft.drawString("Use oculos de protecao!", 120, 260, 2);

    _tft.setTextDatum(TL_DATUM);
}

// ============================================
// Main Screen
// ============================================
void DisplayUI::drawMainScreen(uint8_t power, uint16_t powerWatts, uint8_t speed,
                               const CuttingPreset& preset, uint8_t presetIndex,
                               bool laserOn, bool airOn, bool exhaustOn,
                               LaserMode mode) {
    _tft.fillScreen(COLOR_BG);
    clearButtons();

    _lastPower = power;
    _lastSpeed = speed;
    _lastLaserOn = laserOn;
    _lastMode = mode;

    drawHeader();
    drawPresetSection(preset, presetIndex);
    drawPowerSection(power, powerWatts);
    drawSpeedSection(speed);
    drawControls(laserOn, airOn, exhaustOn);
    drawPowerBar(power);
}

// ============================================
// Header
// ============================================
void DisplayUI::drawHeader() {
    _tft.fillRect(0, 0, 240, 24, COLOR_HEADER_BG);
    _tft.setTextColor(COLOR_NEON_RED, COLOR_HEADER_BG);
    _tft.setTextDatum(TL_DATUM);
    _tft.drawString("KINDLELASER 260W", 5, 4, 2);

    // Mode indicator
    _tft.setTextColor(COLOR_NEON_GREEN, COLOR_HEADER_BG);
    _tft.drawString("[CUT]", 195, 4, 2);

    drawGlowLine(24, COLOR_NEON_RED);
}

// ============================================
// Preset Section
// ============================================
void DisplayUI::drawPresetSection(const CuttingPreset& preset, uint8_t index) {
    _tft.fillRect(0, 26, 240, 24, COLOR_PANEL_BG);

    _tft.setTextColor(COLOR_TEXT_DIM, COLOR_PANEL_BG);
    _tft.setTextDatum(TL_DATUM);
    _tft.drawString("MAT:", 5, 30, 2);

    _tft.setTextColor(COLOR_NEON_CYAN, COLOR_PANEL_BG);

    char presetStr[32];
    snprintf(presetStr, sizeof(presetStr), "%s", preset.name);
    _tft.drawString(presetStr, 45, 30, 2);

    // Gas type
    _tft.setTextColor(COLOR_NEON_ORANGE, COLOR_PANEL_BG);
    char gasStr[10];
    snprintf(gasStr, sizeof(gasStr), "[%s]", preset.gas);
    _tft.drawString(gasStr, 190, 30, 2);

    // Preset number
    _tft.setTextColor(COLOR_TEXT_DIM, COLOR_PANEL_BG);
    char numStr[10];
    snprintf(numStr, sizeof(numStr), "%d/%d", index + 1, PRESET_COUNT);
    _tft.drawString(numStr, 155, 30, 1);
}

// ============================================
// Power Display
// ============================================
void DisplayUI::drawPowerSection(uint8_t power, uint16_t watts) {
    _tft.fillRect(0, 52, 240, 36, COLOR_BG);

    // Label
    _tft.setTextColor(COLOR_TEXT_DIM, COLOR_BG);
    _tft.setTextDatum(TL_DATUM);
    _tft.drawString("POTENCIA", 10, 54, 1);

    // Power value (large)
    uint16_t powerColor = COLOR_NEON_GREEN;
    if (power > 80) powerColor = COLOR_NEON_RED;
    else if (power > 50) powerColor = COLOR_NEON_ORANGE;

    _tft.setTextColor(powerColor, COLOR_BG);
    _tft.setTextDatum(TC_DATUM);

    char powerStr[16];
    snprintf(powerStr, sizeof(powerStr), "%d%%", power);
    _tft.drawString(powerStr, 90, 62, 4);

    // Watts
    _tft.setTextColor(COLOR_TEXT, COLOR_BG);
    char wattsStr[16];
    snprintf(wattsStr, sizeof(wattsStr), "(%dW)", watts);
    _tft.drawString(wattsStr, 180, 68, 2);

    _tft.setTextDatum(TL_DATUM);
}

// ============================================
// Speed Display
// ============================================
void DisplayUI::drawSpeedSection(uint8_t speed) {
    _tft.fillRect(0, 90, 240, 28, COLOR_BG);

    _tft.setTextColor(COLOR_TEXT_DIM, COLOR_BG);
    _tft.setTextDatum(TL_DATUM);
    _tft.drawString("VELOCIDADE", 10, 92, 1);

    _tft.setTextColor(COLOR_SPEED, COLOR_BG);
    _tft.setTextDatum(TC_DATUM);

    char speedStr[16];
    snprintf(speedStr, sizeof(speedStr), "%d mm/s", speed);
    _tft.drawString(speedStr, 140, 94, 2);

    _tft.setTextDatum(TL_DATUM);

    drawGlowLine(118, COLOR_DIVIDER);
}

// ============================================
// Controls
// ============================================
void DisplayUI::drawControls(bool laserOn, bool airOn, bool exhaustOn) {
    int y1 = 122;  // Row 1: Power/Speed
    int y2 = 158;  // Row 2: Material/Test
    int y3 = 194;  // Row 3: Air/Exhaust/Estop
    int y4 = 230;  // Row 4: Laser ON/OFF
    int bh = 32;   // Button height
    int bw = 55;   // Button width

    // Row 1: Power and Speed controls
    drawButton(2, y1, bw, bh, "POT-", COLOR_NEON_ORANGE, false);
    registerButton(2, y1, bw, bh, BTN_POWER_DOWN);

    drawButton(60, y1, bw, bh, "POT+", COLOR_NEON_RED, false);
    registerButton(60, y1, bw, bh, BTN_POWER_UP);

    drawButton(122, y1, bw, bh, "SPD-", COLOR_NEON_BLUE, false);
    registerButton(122, y1, bw, bh, BTN_SPEED_DOWN);

    drawButton(182, y1, bw, bh, "SPD+", COLOR_NEON_CYAN, false);
    registerButton(182, y1, bw, bh, BTN_SPEED_UP);

    // Row 2: Material presets and test pulse
    drawButton(2, y2, 70, bh, "<MAT", COLOR_NEON_CYAN, false);
    registerButton(2, y2, 70, bh, BTN_MATERIAL_PREV);

    drawButton(78, y2, 84, bh, "PULSO", COLOR_NEON_YELLOW, false);
    registerButton(78, y2, 84, bh, BTN_TEST_PULSE);

    drawButton(168, y2, 70, bh, "MAT>", COLOR_NEON_CYAN, false);
    registerButton(168, y2, 70, bh, BTN_MATERIAL_NEXT);

    // Row 3: Air, Exhaust, E-Stop
    drawButton(2, y3, 70, bh, "AR", COLOR_BTN_AIR, airOn);
    registerButton(2, y3, 70, bh, BTN_AIR_TOGGLE);

    drawButton(78, y3, 84, bh, "EXAUST", COLOR_NEON_GREEN, exhaustOn);
    registerButton(78, y3, 84, bh, BTN_EXHAUST_TOGGLE);

    drawButton(168, y3, 70, bh, "STOP", COLOR_NEON_RED, false);
    registerButton(168, y3, 70, bh, BTN_ESTOP);

    // Row 4: Large LASER ON/OFF button
    drawLaserButton(laserOn);
    registerButton(2, y4, 236, bh, BTN_LASER_TOGGLE);
}

void DisplayUI::drawLaserButton(bool laserOn) {
    int y = 230;
    int bh = 32;

    if (laserOn) {
        _tft.fillRoundRect(2, y, 236, bh, 4, COLOR_NEON_RED);
        _tft.drawRoundRect(2, y, 236, bh, 4, COLOR_NEON_WHITE);
        _tft.setTextColor(COLOR_NEON_WHITE, COLOR_NEON_RED);
        _tft.setTextDatum(MC_DATUM);
        _tft.drawString(">>> LASER ATIVO - DESLIGAR <<<", 120, y + bh / 2, 2);
    } else {
        _tft.fillRoundRect(2, y, 236, bh, 4, COLOR_BTN_BG);
        _tft.drawRoundRect(2, y, 236, bh, 4, COLOR_NEON_GREEN);
        _tft.setTextColor(COLOR_NEON_GREEN, COLOR_BTN_BG);
        _tft.setTextDatum(MC_DATUM);
        _tft.drawString("LASER ON/OFF", 120, y + bh / 2, 2);
    }
    _tft.setTextDatum(TL_DATUM);
}

// ============================================
// Power Bar
// ============================================
void DisplayUI::drawPowerBar(uint8_t power) {
    int y = 266;
    int barW = 200;
    int barH = 14;
    int x = 5;

    // Background
    _tft.fillRect(x, y, barW, barH, COLOR_BAR_BG);

    // Fill
    int fillW = (power * barW) / 100;
    uint16_t barColor = COLOR_NEON_GREEN;
    if (power > 80) barColor = COLOR_NEON_RED;
    else if (power > 50) barColor = COLOR_NEON_ORANGE;

    _tft.fillRect(x, y, fillW, barH, barColor);

    // Border
    _tft.drawRect(x, y, barW, barH, COLOR_DIVIDER);

    // Percentage text
    _tft.setTextColor(COLOR_TEXT_BRIGHT, COLOR_BG);
    char pctStr[8];
    snprintf(pctStr, sizeof(pctStr), "%d%%", power);
    _tft.drawString(pctStr, 210, y + 1, 2);
}

// ============================================
// Safety Bar
// ============================================
void DisplayUI::drawSafetyBar(bool waterOK, bool lidClosed, bool estopOK, float temp) {
    int y = 284;
    _tft.fillRect(0, y, 240, 36, COLOR_HEADER_BG);

    _tft.setTextDatum(TL_DATUM);

    // Water flow
    _tft.setTextColor(waterOK ? COLOR_SAFE : COLOR_DANGER, COLOR_HEADER_BG);
    _tft.drawString(waterOK ? "H2O:OK" : "H2O:NO", 5, y + 4, 1);

    // Lid switch
    _tft.setTextColor(lidClosed ? COLOR_SAFE : COLOR_DANGER, COLOR_HEADER_BG);
    _tft.drawString(lidClosed ? "LID:OK" : "LID:OPEN", 55, y + 4, 1);

    // E-stop
    _tft.setTextColor(estopOK ? COLOR_SAFE : COLOR_DANGER, COLOR_HEADER_BG);
    _tft.drawString(estopOK ? "ESTOP:OK" : "ESTOP!", 115, y + 4, 1);

    // Temperature
    uint16_t tempColor = COLOR_SAFE;
    if (temp >= WATER_TEMP_MAX) tempColor = COLOR_DANGER;
    else if (temp >= WATER_TEMP_WARNING) tempColor = COLOR_WARNING;

    _tft.setTextColor(tempColor, COLOR_HEADER_BG);
    char tempStr[16];
    snprintf(tempStr, sizeof(tempStr), "%.0fC", temp);
    _tft.drawString(tempStr, 185, y + 4, 1);

    // Overall safety status
    bool allOK = waterOK && lidClosed && estopOK && (temp < WATER_TEMP_MAX);
    _tft.setTextColor(allOK ? COLOR_SAFE : COLOR_DANGER, COLOR_HEADER_BG);
    _tft.drawString(allOK ? "SEGURO" : "BLOQUEADO", 5, y + 20, 2);

    // Divider
    drawGlowLine(y, allOK ? COLOR_NEON_GREEN : COLOR_NEON_RED);
}

// ============================================
// Partial Update Methods
// ============================================
void DisplayUI::updatePower(uint8_t power, uint16_t powerWatts) {
    if (power != _lastPower) {
        _lastPower = power;
        drawPowerSection(power, powerWatts);
        drawPowerBar(power);
    }
}

void DisplayUI::updateSpeed(uint8_t speed) {
    if (speed != _lastSpeed) {
        _lastSpeed = speed;
        drawSpeedSection(speed);
    }
}

void DisplayUI::updateLaserStatus(bool laserOn, LaserMode mode) {
    if (laserOn != _lastLaserOn || mode != _lastMode) {
        _lastLaserOn = laserOn;
        _lastMode = mode;
        drawLaserButton(laserOn);

        // Update header status indicator
        _tft.fillRect(170, 4, 65, 16, COLOR_HEADER_BG);
        uint16_t modeColor = COLOR_IDLE;
        const char* modeStr = "[IDLE]";

        switch (mode) {
        case MODE_READY:
            modeColor = COLOR_SAFE;
            modeStr = "[OK]";
            break;
        case MODE_CUTTING:
            modeColor = COLOR_NEON_RED;
            modeStr = "[CUT!]";
            break;
        case MODE_ERROR:
            modeColor = COLOR_DANGER;
            modeStr = "[ERR]";
            break;
        case MODE_PAUSED:
            modeColor = COLOR_WARNING;
            modeStr = "[PAUSE]";
            break;
        default:
            break;
        }

        _tft.setTextColor(modeColor, COLOR_HEADER_BG);
        _tft.drawString(modeStr, 192, 4, 2);
    }
}

void DisplayUI::updateAirStatus(bool airOn) {
    drawButton(2, 194, 70, 32, "AR", COLOR_BTN_AIR, airOn);
}

void DisplayUI::updateExhaustStatus(bool exhaustOn) {
    drawButton(78, 194, 84, 32, "EXAUST", COLOR_NEON_GREEN, exhaustOn);
}

void DisplayUI::updatePreset(const CuttingPreset& preset, uint8_t presetIndex) {
    drawPresetSection(preset, presetIndex);
}

void DisplayUI::updateCuttingTime(unsigned long ms) {
    // Show cutting time in safety bar area
    unsigned long sec = ms / 1000;
    unsigned long min = sec / 60;
    sec = sec % 60;

    _tft.fillRect(170, 300, 70, 16, COLOR_HEADER_BG);
    _tft.setTextColor(COLOR_NEON_CYAN, COLOR_HEADER_BG);

    char timeStr[12];
    snprintf(timeStr, sizeof(timeStr), "%02lu:%02lu", min, sec);
    _tft.drawString(timeStr, 185, 302, 2);
}

void DisplayUI::updateSafety(bool waterOK, bool lidClosed, bool estopOK,
                              float waterTemp, bool tempWarning) {
    drawSafetyBar(waterOK, lidClosed, estopOK, waterTemp);
}

// ============================================
// Messages / Errors
// ============================================
void DisplayUI::showMessage(const char* msg, uint16_t color) {
    // Temporary message overlay
    _tft.fillRect(10, 140, 220, 30, COLOR_PANEL_BG);
    _tft.drawRect(10, 140, 220, 30, color);
    _tft.setTextColor(color, COLOR_PANEL_BG);
    _tft.setTextDatum(MC_DATUM);
    _tft.drawString(msg, 120, 155, 2);
    _tft.setTextDatum(TL_DATUM);
}

void DisplayUI::showError(const char* msg) {
    showMessage(msg, COLOR_DANGER);
}

// ============================================
// Touch Handling
// ============================================
TouchButton DisplayUI::checkTouch() {
    unsigned long now = millis();
    if (now - _lastTouchTime < TOUCH_DEBOUNCE_MS) return BTN_NONE;

    int16_t x, y;
    if (!getTouchPoint(x, y)) return BTN_NONE;

    _lastTouchTime = now;
    return hitTest(x, y);
}

bool DisplayUI::getTouchPoint(int16_t &x, int16_t &y) {
    uint16_t tx, ty;
    if (!_tft.getTouch(&tx, &ty)) return false;

    // Map raw touch to screen coordinates
    x = map(tx, TOUCH_MIN_X, TOUCH_MAX_X, 0, SCREEN_WIDTH);
    y = map(ty, TOUCH_MIN_Y, TOUCH_MAX_Y, 0, SCREEN_HEIGHT);

    // Clamp
    if (x < 0) x = 0;
    if (x >= SCREEN_WIDTH) x = SCREEN_WIDTH - 1;
    if (y < 0) y = 0;
    if (y >= SCREEN_HEIGHT) y = SCREEN_HEIGHT - 1;

    return true;
}

TouchButton DisplayUI::hitTest(int16_t x, int16_t y) {
    for (int i = 0; i < _buttonCount; i++) {
        TouchArea& btn = _buttons[i];
        if (x >= btn.x && x < btn.x + btn.w &&
            y >= btn.y && y < btn.y + btn.h) {
            return btn.id;
        }
    }
    return BTN_NONE;
}

// ============================================
// Button Drawing
// ============================================
void DisplayUI::drawButton(int16_t x, int16_t y, int16_t w, int16_t h,
                           const char* label, uint16_t glowColor, bool active) {
    uint16_t bg = active ? glowColor : COLOR_BTN_BG;
    uint16_t textColor = active ? COLOR_BG : glowColor;

    _tft.fillRoundRect(x, y, w, h, 3, bg);
    _tft.drawRoundRect(x, y, w, h, 3, glowColor);

    _tft.setTextColor(textColor, bg);
    _tft.setTextDatum(MC_DATUM);
    _tft.drawString(label, x + w / 2, y + h / 2, 1);
    _tft.setTextDatum(TL_DATUM);
}

// ============================================
// Glow Line
// ============================================
void DisplayUI::drawGlowLine(int y, uint16_t color) {
    _tft.drawFastHLine(0, y, 240, color);
}

// ============================================
// Button Management
// ============================================
void DisplayUI::registerButton(int16_t x, int16_t y, int16_t w, int16_t h, TouchButton id) {
    if (_buttonCount >= MAX_BUTTONS) return;
    _buttons[_buttonCount] = {x, y, w, h, id};
    _buttonCount++;
}

void DisplayUI::clearButtons() {
    _buttonCount = 0;
}
