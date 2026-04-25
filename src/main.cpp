/**
 * ESP32 Radio Bluetooth - Pioneer Style
 *
 * Firmware principal - controle 100% por tela touch
 *  - Módulo FM TEA5767 (I2C)
 *  - Display TFT ILI9341 2.4" 240x320 com Touch XPT2046 (SPI)
 *  - Bluetooth A2DP Sink para conexão com Alexa
 *  - LED WS2812B reativo ao som
 *  - Visual estilo Pioneer automotivo
 */

#include <Arduino.h>
#include "config.h"
#include "display_ui.h"
#include "bt_audio.h"
#include "led_effects.h"
#include "TEA5767.h"

// ============================================
// Global Objects
// ============================================
TEA5767     radio;
DisplayUI   display;
BTAudio     btAudio;
LEDEffects  ledStrip;

RadioMode   currentMode = MODE_FM;
uint8_t     volume = VOLUME_DEFAULT;
bool        muted = false;
bool        btStarted = false;

// ============================================
// Timing
// ============================================
unsigned long lastStatusRead = 0;

// ============================================
// Bluetooth Callbacks
// ============================================
void onBTConnection(bool connected) {
    if (currentMode == MODE_BLUETOOTH) {
        display.updateBTStatus(connected, btAudio.getConnectedDeviceName());
    }
}

void onBTPlayback(bool playing) {
    if (currentMode == MODE_BLUETOOTH) {
        display.updatePlayStatus(playing);
    }
}

// ============================================
// Mode Switching
// ============================================
void switchToFM() {
    currentMode = MODE_FM;
    Serial.println("[MAIN] Modo FM");

    if (btStarted) {
        btAudio.end();
        btStarted = false;
        delay(500);
    }

    radio.setStandby(false);

    TEA5767_Status status = radio.getStatus();
    display.drawFMScreen(
        radio.getFrequency(),
        status.stereo,
        status.signalLevel,
        muted,
        radio.getCurrentPresetIndex(),
        radio.getPresetCount(),
        volume,
        ledStrip.getEffectName()
    );
}

void switchToBluetooth() {
    currentMode = MODE_BLUETOOTH;
    Serial.println("[MAIN] Modo Bluetooth");

    radio.setStandby(true);

    if (!btStarted) {
        btAudio.begin(BT_DEVICE_NAME);
        btAudio.setConnectionCallback(onBTConnection);
        btAudio.setPlaybackCallback(onBTPlayback);
        btAudio.setVolume(volume);
        btStarted = true;
    }

    display.drawBluetoothScreen(
        btAudio.isConnected(),
        btAudio.getConnectedDeviceName(),
        muted,
        volume,
        btAudio.isPlaying(),
        ledStrip.getEffectName()
    );
}

// ============================================
// Volume Control
// ============================================
void adjustVolume(int delta) {
    int newVol = (int)volume + delta;
    if (newVol < 0) newVol = 0;
    if (newVol > VOLUME_MAX) newVol = VOLUME_MAX;
    volume = (uint8_t)newVol;

    if (currentMode == MODE_BLUETOOTH && btStarted) {
        btAudio.setVolume(volume);
    }

    display.updateVolume(volume);
    Serial.printf("[MAIN] Volume: %d%%\n", volume);
}

void toggleMute() {
    muted = !muted;

    if (currentMode == MODE_FM) {
        radio.setMute(muted);
    }

    display.updateMuteIndicator(muted);
    Serial.printf("[MAIN] Mute: %s\n", muted ? "ON" : "OFF");
}

// ============================================
// Touch Handling
// ============================================
void handleTouch() {
    TouchButton btn = display.checkTouch();
    if (btn == BTN_NONE) return;

    switch (btn) {
    case BTN_MODE:
        if (currentMode == MODE_FM) switchToBluetooth();
        else switchToFM();
        break;

    case BTN_SEEK_UP:
        if (currentMode == MODE_FM) {
            display.showSeekingAnimation();
            radio.seekUp();
            TEA5767_Status s = radio.getStatus();
            display.updateFrequency(radio.getFrequency());
            display.updateSignalLevel(s.signalLevel);
            display.updateStereoIndicator(s.stereo);
            Serial.printf("[FM] Seek >> %.1f MHz\n", radio.getFrequency());
        }
        break;

    case BTN_SEEK_DOWN:
        if (currentMode == MODE_FM) {
            display.showSeekingAnimation();
            radio.seekDown();
            TEA5767_Status s = radio.getStatus();
            display.updateFrequency(radio.getFrequency());
            display.updateSignalLevel(s.signalLevel);
            display.updateStereoIndicator(s.stereo);
            Serial.printf("[FM] Seek << %.1f MHz\n", radio.getFrequency());
        }
        break;

    case BTN_PRESET_NEXT:
        if (currentMode == MODE_FM) {
            radio.nextPreset();
            display.updateFrequency(radio.getFrequency());
            display.updatePresetInfo(
                radio.getCurrentPresetIndex(),
                radio.getFrequency(),
                radio.getPresetCount()
            );
            Serial.printf("[FM] Preset >> %.1f MHz\n", radio.getFrequency());
        }
        break;

    case BTN_PRESET_PREV:
        if (currentMode == MODE_FM) {
            radio.prevPreset();
            display.updateFrequency(radio.getFrequency());
            display.updatePresetInfo(
                radio.getCurrentPresetIndex(),
                radio.getFrequency(),
                radio.getPresetCount()
            );
            Serial.printf("[FM] Preset << %.1f MHz\n", radio.getFrequency());
        }
        break;

    case BTN_VOL_UP:
        adjustVolume(VOLUME_STEP);
        break;

    case BTN_VOL_DOWN:
        adjustVolume(-VOLUME_STEP);
        break;

    case BTN_MUTE:
        toggleMute();
        break;

    case BTN_PLAY_PAUSE:
        if (currentMode == MODE_BLUETOOTH && btStarted) {
            if (btAudio.isPlaying()) btAudio.pause();
            else btAudio.play();
        }
        break;

    case BTN_LED_MODE:
        ledStrip.nextEffect();
        display.showMessage(ledStrip.getEffectName(), COLOR_NEON_MAGENTA);
        break;

    case BTN_LED_BRIGHT:
        ledStrip.cycleBrightness();
        {
            char msg[20];
            snprintf(msg, sizeof(msg), "LED: %d%%", ledStrip.getBrightness() * 100 / 255);
            display.showMessage(msg, COLOR_NEON_MAGENTA);
        }
        break;

    default:
        break;
    }
}

// ============================================
// Periodic Status Update
// ============================================
void updateRadioStatus() {
    if (currentMode != MODE_FM) return;

    TEA5767_Status status = radio.getStatus();
    display.updateSignalLevel(status.signalLevel);
    display.updateStereoIndicator(status.stereo);
}

// ============================================
// Setup
// ============================================
void setup() {
    Serial.begin(115200);
    Serial.println("\n=== ESP32 Radio - Pioneer Style ===");
    Serial.println("Touch + LED WS2812B reativo ao som");

    // Initialize display + touch
    Serial.println("[INIT] Display TFT ILI9341 + Touch...");
    display.begin();
    display.drawSplashScreen();

    // Initialize FM radio
    Serial.println("[INIT] Radio FM TEA5767...");
    bool radioOk = radio.begin(I2C_SDA, I2C_SCL);
    if (radioOk) {
        Serial.println("[INIT] TEA5767 OK");
    } else {
        Serial.println("[INIT] TEA5767 FALHA - verifique I2C");
        display.showMessage("TEA5767 nao encontrado!", COLOR_NEON_RED);
        delay(2000);
    }

    // Initialize LED strip
    Serial.println("[INIT] LED WS2812B...");
    ledStrip.begin();

    // Start in FM mode
    switchToFM();

    Serial.println("[INIT] Sistema pronto!");
}

// ============================================
// Main Loop
// ============================================
void loop() {
    unsigned long now = millis();

    // Handle touch input
    handleTouch();

    // Update EQ visualizer on display
    display.updateEQ();

    // Update LED strip (reads audio + applies effect)
    ledStrip.update();

    // Update radio status periodically
    if (now - lastStatusRead >= STATUS_READ_MS) {
        lastStatusRead = now;
        updateRadioStatus();
    }

    delay(5);
}
