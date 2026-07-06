/**
 * ESP32 Kindlelaser Max 260W - Metal Cutting Controller
 *
 * Painel de controle touch para corte de metal com laser CO2
 *  - Display TFT ILI9341 2.4" com Touch XPT2046 (SPI)
 *  - Controle de potência PWM (0-99%)
 *  - Controle de velocidade de corte (mm/s)
 *  - Presets de materiais (Aço, Inox, Alumínio)
 *  - Monitoramento de segurança (água, tampa, temperatura, E-stop)
 *  - LED WS2812B para indicação de status
 *  - Comunicação UART com controladora Ruida (opcional)
 *
 * SEGURANÇA:
 *  - Laser NUNCA liga sem fluxo de água confirmado
 *  - Laser DESLIGA se tampa abrir durante operação
 *  - E-STOP desliga TUDO imediatamente
 *  - Temperatura da água monitorada continuamente
 *  - Tempo máximo de corte contínuo: 5 minutos
 */

#include <Arduino.h>
#include "config.h"
#include "display_ui.h"
#include "laser_control.h"
#include "led_effects.h"

// ============================================
// Global Objects
// ============================================
LaserControl laser;
DisplayUI    display;
LEDEffects   ledStrip;

// ============================================
// Timing
// ============================================
unsigned long lastDisplayUpdate = 0;
unsigned long lastStatusUpdate = 0;

// ============================================
// Touch Handling
// ============================================
void handleTouch() {
    TouchButton btn = display.checkTouch();
    if (btn == BTN_NONE) return;

    switch (btn) {
    case BTN_POWER_UP:
        laser.adjustPower(POWER_STEP);
        display.updatePower(laser.getPower(), laser.getPowerWatts());
        break;

    case BTN_POWER_DOWN:
        laser.adjustPower(-POWER_STEP);
        display.updatePower(laser.getPower(), laser.getPowerWatts());
        break;

    case BTN_SPEED_UP:
        laser.adjustSpeed(SPEED_STEP);
        display.updateSpeed(laser.getSpeed());
        break;

    case BTN_SPEED_DOWN:
        laser.adjustSpeed(-SPEED_STEP);
        display.updateSpeed(laser.getSpeed());
        break;

    case BTN_LASER_TOGGLE:
        if (laser.isLaserOn()) {
            laser.laserOff();
        } else {
            laser.laserOn();
        }
        display.updateLaserStatus(laser.isLaserOn(), laser.getMode());
        break;

    case BTN_AIR_TOGGLE:
        laser.toggleAirAssist();
        display.updateAirStatus(laser.isAirAssistOn());
        break;

    case BTN_MATERIAL_NEXT:
        laser.nextPreset();
        display.updatePreset(laser.getCurrentPreset(), laser.getCurrentPresetIndex());
        display.updatePower(laser.getPower(), laser.getPowerWatts());
        display.updateSpeed(laser.getSpeed());
        break;

    case BTN_MATERIAL_PREV:
        laser.prevPreset();
        display.updatePreset(laser.getCurrentPreset(), laser.getCurrentPresetIndex());
        display.updatePower(laser.getPower(), laser.getPowerWatts());
        display.updateSpeed(laser.getSpeed());
        break;

    case BTN_TEST_PULSE:
        if (!laser.isLaserOn()) {
            display.showMessage("PULSO TESTE...", COLOR_NEON_YELLOW);
            laser.testPulse(100);
            display.showMessage("PULSO OK", COLOR_NEON_GREEN);
        }
        break;

    case BTN_EXHAUST_TOGGLE:
        laser.toggleExhaust();
        display.updateExhaustStatus(laser.isExhaustOn());
        break;

    case BTN_ESTOP:
        laser.emergencyStop();
        display.showError("PARADA DE EMERGENCIA!");
        break;

    default:
        break;
    }
}

// ============================================
// Safety Status Update
// ============================================
void updateSafetyDisplay() {
    display.updateSafety(
        laser.isWaterFlowOK(),
        laser.isLidClosed(),
        laser.isEstopOK(),
        laser.getWaterTemp(),
        laser.isTempWarning()
    );

    // Update mode display
    display.updateLaserStatus(laser.isLaserOn(), laser.getMode());

    // Show error message if in error mode
    if (laser.getMode() == MODE_ERROR) {
        display.showError(laser.getErrorMessage());
    }
}

// ============================================
// LED Status Update
// ============================================
void updateLEDStatus() {
    LEDStatus status;

    switch (laser.getMode()) {
    case MODE_IDLE:
        status = LED_STATUS_IDLE;
        break;
    case MODE_READY:
        status = LED_STATUS_READY;
        break;
    case MODE_CUTTING:
        status = LED_STATUS_CUTTING;
        break;
    case MODE_WARMUP:
        status = LED_STATUS_READY;
        break;
    case MODE_PAUSED:
        status = LED_STATUS_WARNING;
        break;
    case MODE_ERROR:
        status = LED_STATUS_ERROR;
        break;
    default:
        status = LED_STATUS_OFF;
        break;
    }

    ledStrip.setStatus(status);
}

// ============================================
// Setup
// ============================================
void setup() {
    Serial.begin(115200);
    Serial.println("\n=== ESP32 Kindlelaser Max 260W Controller ===");
    Serial.println("Painel de controle touch para corte de metal");

    // Initialize display + touch
    Serial.println("[INIT] Display TFT ILI9341 + Touch...");
    display.begin();
    display.drawSplashScreen();

    // Initialize laser control
    Serial.println("[INIT] Laser control system...");
    laser.begin();

    // Initialize LED strip
    Serial.println("[INIT] LED status strip...");
    ledStrip.begin();

    // Load first preset
    laser.loadPreset(0);

    // Draw main screen
    delay(2000); // Show splash
    display.drawMainScreen(
        laser.getPower(),
        laser.getPowerWatts(),
        laser.getSpeed(),
        laser.getCurrentPreset(),
        laser.getCurrentPresetIndex(),
        laser.isLaserOn(),
        laser.isAirAssistOn(),
        laser.isExhaustOn(),
        laser.getMode()
    );

    // Initial safety display
    updateSafetyDisplay();

    Serial.println("[INIT] Sistema pronto!");
    Serial.println("[INIT] Aguardando condições de segurança...");
}

// ============================================
// Main Loop
// ============================================
void loop() {
    unsigned long now = millis();

    // Handle touch input
    handleTouch();

    // Update laser safety (critical - runs every cycle)
    laser.update();

    // Update LED status strip
    ledStrip.update();
    updateLEDStatus();

    // Update display periodically
    if (now - lastDisplayUpdate >= DISPLAY_UPDATE_MS) {
        lastDisplayUpdate = now;

        // Update cutting time if active
        if (laser.getMode() == MODE_CUTTING) {
            display.updateCuttingTime(laser.getCuttingTime());
        }
    }

    // Update safety status display
    if (now - lastStatusUpdate >= STATUS_READ_MS) {
        lastStatusUpdate = now;
        updateSafetyDisplay();
    }

    delay(5);
}
