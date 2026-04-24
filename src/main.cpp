/**
 * ESP32 Radio Bluetooth
 * 
 * Firmware principal que integra:
 *  - Módulo FM TEA5767 (I2C)
 *  - Display TFT ILI9341 2.4" 240x320 (SPI)
 *  - Bluetooth A2DP Sink para conexão com Alexa
 * 
 * Modos de operação:
 *  - FM: sintoniza estações FM com busca automática e presets
 *  - Bluetooth: recebe áudio da Alexa ou qualquer dispositivo BT
 */

#include <Arduino.h>
#include "config.h"
#include "display_ui.h"
#include "bt_audio.h"
#include "TEA5767.h"

// ============================================
// Global Objects
// ============================================
TEA5767     radio;
DisplayUI   display;
BTAudio     btAudio;

RadioMode   currentMode = MODE_FM;
uint8_t     volume = VOLUME_DEFAULT;
bool        muted = false;
bool        btStarted = false;

// ============================================
// Timing
// ============================================
unsigned long lastDisplayUpdate = 0;
unsigned long lastStatusRead = 0;
unsigned long lastButtonCheck = 0;

// ============================================
// Button state
// ============================================
struct Button {
    uint8_t pin;
    bool lastState;
    unsigned long lastPress;
};

Button buttons[] = {
    {BTN_MODE,        HIGH, 0},
    {BTN_SEEK_UP,     HIGH, 0},
    {BTN_SEEK_DOWN,   HIGH, 0},
    {BTN_MUTE,        HIGH, 0},
    {BTN_PRESET_NEXT, HIGH, 0},
    {BTN_PRESET_PREV, HIGH, 0}
};
const int NUM_BUTTONS = sizeof(buttons) / sizeof(buttons[0]);

// ============================================
// Encoder state (optional)
// ============================================
volatile int encoderPos = 0;
int lastEncoderPos = 0;

void IRAM_ATTR encoderISR() {
    static uint8_t lastState = 0;
    uint8_t clk = digitalRead(ENCODER_CLK);
    uint8_t dt = digitalRead(ENCODER_DT);
    uint8_t state = (clk << 1) | dt;

    if (lastState == 0b00) {
        if (state == 0b01) encoderPos++;
        if (state == 0b10) encoderPos--;
    }
    lastState = state;
}

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
    Serial.println("[MAIN] Switching to FM mode");

    // Stop Bluetooth if running
    if (btStarted) {
        btAudio.end();
        btStarted = false;
        delay(500);
    }

    // Activate radio
    radio.setStandby(false);

    // Redraw screen
    TEA5767_Status status = radio.getStatus();
    display.drawFMScreen(
        radio.getFrequency(),
        status.stereo,
        status.signalLevel,
        muted,
        radio.getCurrentPresetIndex(),
        radio.getPresetCount(),
        volume
    );
}

void switchToBluetooth() {
    currentMode = MODE_BLUETOOTH;
    Serial.println("[MAIN] Switching to Bluetooth mode");

    // Put radio in standby
    radio.setStandby(true);

    // Start Bluetooth
    if (!btStarted) {
        btAudio.begin(BT_DEVICE_NAME);
        btAudio.setConnectionCallback(onBTConnection);
        btAudio.setPlaybackCallback(onBTPlayback);
        btAudio.setVolume(volume);
        btStarted = true;
    }

    // Redraw screen
    display.drawBluetoothScreen(
        btAudio.isConnected(),
        btAudio.getConnectedDeviceName(),
        muted,
        volume,
        btAudio.isPlaying()
    );
}

void toggleMode() {
    if (currentMode == MODE_FM) {
        switchToBluetooth();
    } else {
        switchToFM();
    }
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
// Button Handling
// ============================================
bool isButtonPressed(int index) {
    bool currentState = digitalRead(buttons[index].pin);
    if (currentState == LOW && buttons[index].lastState == HIGH) {
        if (millis() - buttons[index].lastPress > DEBOUNCE_MS) {
            buttons[index].lastPress = millis();
            buttons[index].lastState = currentState;
            return true;
        }
    }
    buttons[index].lastState = currentState;
    return false;
}

void handleButtons() {
    // Mode button
    if (isButtonPressed(0)) {
        toggleMode();
    }

    if (currentMode == MODE_FM) {
        // Seek Up
        if (isButtonPressed(1)) {
            display.showSeekingAnimation();
            radio.seekUp();
            TEA5767_Status status = radio.getStatus();
            display.updateFrequency(radio.getFrequency());
            display.updateSignalLevel(status.signalLevel);
            display.updateStereoIndicator(status.stereo);
            Serial.printf("[FM] Seek Up -> %.1f MHz\n", radio.getFrequency());
        }

        // Seek Down
        if (isButtonPressed(2)) {
            display.showSeekingAnimation();
            radio.seekDown();
            TEA5767_Status status = radio.getStatus();
            display.updateFrequency(radio.getFrequency());
            display.updateSignalLevel(status.signalLevel);
            display.updateStereoIndicator(status.stereo);
            Serial.printf("[FM] Seek Down -> %.1f MHz\n", radio.getFrequency());
        }

        // Preset Next
        if (isButtonPressed(4)) {
            radio.nextPreset();
            TEA5767_Status status = radio.getStatus();
            display.updateFrequency(radio.getFrequency());
            display.updatePresetInfo(
                radio.getCurrentPresetIndex(),
                radio.getFrequency(),
                radio.getPresetCount()
            );
            Serial.printf("[FM] Preset -> %.1f MHz\n", radio.getFrequency());
        }

        // Preset Prev
        if (isButtonPressed(5)) {
            radio.prevPreset();
            TEA5767_Status status = radio.getStatus();
            display.updateFrequency(radio.getFrequency());
            display.updatePresetInfo(
                radio.getCurrentPresetIndex(),
                radio.getFrequency(),
                radio.getPresetCount()
            );
            Serial.printf("[FM] Preset <- %.1f MHz\n", radio.getFrequency());
        }
    } else {
        // Volume Up in BT mode
        if (isButtonPressed(1)) {
            adjustVolume(VOLUME_STEP);
        }

        // Volume Down in BT mode
        if (isButtonPressed(2)) {
            adjustVolume(-VOLUME_STEP);
        }
    }

    // Mute (both modes)
    if (isButtonPressed(3)) {
        toggleMute();
    }
}

// ============================================
// Encoder Handling (optional fine-tuning)
// ============================================
void handleEncoder() {
    int pos = encoderPos;
    int delta = pos - lastEncoderPos;

    if (delta == 0) return;
    lastEncoderPos = pos;

    if (currentMode == MODE_FM) {
        float newFreq = radio.getFrequency() + (delta * FREQ_STEP);
        radio.setFrequency(newFreq);
        display.updateFrequency(radio.getFrequency());
        Serial.printf("[FM] Tuning -> %.1f MHz\n", radio.getFrequency());
    } else {
        adjustVolume(delta * VOLUME_STEP);
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
    Serial.println("\n=== ESP32 Radio Bluetooth ===");
    Serial.println("Initializing...");

    // Initialize buttons with pull-up
    for (int i = 0; i < NUM_BUTTONS; i++) {
        pinMode(buttons[i].pin, INPUT_PULLUP);
    }

    // Encoder pins (optional)
    pinMode(ENCODER_CLK, INPUT_PULLUP);
    pinMode(ENCODER_DT, INPUT_PULLUP);
    pinMode(ENCODER_SW, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(ENCODER_CLK), encoderISR, CHANGE);

    // Initialize display
    Serial.println("[INIT] Display TFT ILI9341...");
    display.begin();
    display.drawSplashScreen();

    // Initialize FM radio
    Serial.println("[INIT] Radio FM TEA5767...");
    bool radioOk = radio.begin(I2C_SDA, I2C_SCL);
    if (radioOk) {
        Serial.println("[INIT] TEA5767 OK");
    } else {
        Serial.println("[INIT] TEA5767 FALHA - verifique conexao I2C");
        display.showMessage("TEA5767 nao encontrado!", COLOR_MUTED);
        delay(2000);
    }

    // Start in FM mode
    switchToFM();

    Serial.println("[INIT] Sistema pronto!");
    Serial.println("[INIT] Pressione MODE para alternar FM/BT");
}

// ============================================
// Main Loop
// ============================================
void loop() {
    unsigned long now = millis();

    // Handle buttons
    handleButtons();

    // Handle encoder
    handleEncoder();

    // Update radio status periodically
    if (now - lastStatusRead >= STATUS_READ_MS) {
        lastStatusRead = now;
        updateRadioStatus();
    }

    // Small delay to prevent watchdog
    delay(10);
}
