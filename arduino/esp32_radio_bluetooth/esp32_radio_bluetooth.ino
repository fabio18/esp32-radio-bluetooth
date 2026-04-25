/**
 * ====================================================
 * ESP32 Radio Bluetooth - Pioneer Style
 * ====================================================
 * Visual automotivo + LED WS2812B reativo ao som
 * Controle 100% por tela touch (sem botões físicos)
 *
 * COMPONENTES:
 *  - ESP32 DevKit
 *  - TEA5767 FM Radio (I2C)
 *  - ILI9341 TFT 2.4" 240x320 com Touch XPT2046 (SPI)
 *  - WS2812B LED Strip (30 LEDs)
 *  - DAC I2S (MAX98357A ou PCM5102)
 *
 * ARDUINO IDE - INSTRUÇÕES:
 *  1. Instale o suporte ESP32 no Boards Manager
 *  2. Instale as bibliotecas:
 *     - TFT_eSPI by Bodmer
 *     - FastLED by Daniel Garcia
 *     - ESP32-A2DP by Phil Schatzmann
 *  3. IMPORTANTE: Copie o arquivo User_Setup.h para a pasta
 *     da biblioteca TFT_eSPI (substituindo o existente):
 *     Arduino/libraries/TFT_eSPI/User_Setup.h
 *  4. Selecione a placa: ESP32 Dev Module
 *  5. Partition Scheme: Huge APP (3MB No OTA/1MB SPIFFS)
 *  6. Upload Speed: 921600
 *
 * ====================================================
 */

#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <FastLED.h>
#include "BluetoothA2DPSink.h"

// ============================================
// CONFIGURAÇÃO DE PINOS
// ============================================
#define I2C_SDA         21
#define I2C_SCL         22
#define I2S_BCLK        5
#define I2S_LRC         17
#define I2S_DOUT        16
#define TOUCH_CS        27
#define TOUCH_IRQ       34
#define LED_PIN         13
#define LED_COUNT       30
#define LED_BRIGHTNESS  150
#define AUDIO_IN_PIN    36

// ============================================
// CONSTANTES
// ============================================
#define SCREEN_WIDTH    240
#define SCREEN_HEIGHT   320
#define BT_DEVICE_NAME  "ESP32 Radio"
#define TOUCH_DEBOUNCE_MS   300
#define DISPLAY_UPDATE_MS   100
#define STATUS_READ_MS      500
#define EQ_UPDATE_MS        80
#define LED_UPDATE_MS       20
#define VOLUME_DEFAULT      80
#define VOLUME_MAX          100
#define VOLUME_STEP         5
#define EQ_BARS             16
#define EQ_MAX_HEIGHT       30
#define AUDIO_SAMPLES       64
#define AUDIO_NOISE_FLOOR   50

// Touch calibration
#define TOUCH_MIN_X     300
#define TOUCH_MAX_X     3800
#define TOUCH_MIN_Y     300
#define TOUCH_MAX_Y     3800

// ============================================
// CORES PIONEER (RGB565)
// ============================================
#define COLOR_BG            0x0001
#define COLOR_HEADER_BG     0x0011
#define COLOR_BTN_BG        0x0841
#define COLOR_BTN_PRESSED   0x1082
#define COLOR_NEON_CYAN     0x07FF
#define COLOR_NEON_BLUE     0x03BF
#define COLOR_NEON_GREEN    0x07E0
#define COLOR_NEON_ORANGE   0xFCA0
#define COLOR_NEON_RED      0xF800
#define COLOR_NEON_MAGENTA  0xF81F
#define COLOR_NEON_WHITE    0xFFFF
#define COLOR_FREQ          0x07FF
#define COLOR_FREQ_GLOW     0x033F
#define COLOR_TEXT          0xBDF7
#define COLOR_TEXT_DIM      0x52AA
#define COLOR_TEXT_BRIGHT   0xFFFF
#define COLOR_DIVIDER       0x0842
#define COLOR_SIGNAL_HIGH   0x07E0
#define COLOR_SIGNAL_LOW    0xFCA0
#define COLOR_STEREO        0x07FF
#define COLOR_BTN_BORDER    0x02DF
#define COLOR_BTN_MODE_FM   0x07E0
#define COLOR_BTN_MODE_BT   0x03BF
#define COLOR_VOL_BAR       0x07E0
#define COLOR_VOL_HIGH      0xFCA0
#define COLOR_VOL_MAX       0xF800
#define COLOR_VOL_BG        0x1082
#define COLOR_EQ_LOW        0x07E0
#define COLOR_EQ_MID        0x07FF
#define COLOR_EQ_HIGH       0x03BF
#define COLOR_EQ_PEAK       0xFFE0
#define COLOR_EQ_BG         0x0841
#define COLOR_BT_ICON       0x03BF
#define COLOR_BT_CONNECTED  0x07E0
#define COLOR_BT_WAITING    0xFCA0

// ============================================
// ENUMS
// ============================================
enum RadioMode { MODE_FM = 0, MODE_BLUETOOTH = 1 };

enum TouchButton {
  BTN_NONE = 0, BTN_SEEK_DOWN, BTN_SEEK_UP,
  BTN_PRESET_PREV, BTN_PRESET_NEXT,
  BTN_VOL_DOWN, BTN_VOL_UP, BTN_MUTE,
  BTN_MODE, BTN_PLAY_PAUSE,
  BTN_LED_MODE, BTN_LED_BRIGHT
};

enum LEDEffect {
  LED_OFF = 0, LED_VU_METER, LED_SPECTRUM,
  LED_PULSE, LED_RAINBOW, LED_FIRE, LED_EFFECT_COUNT
};

struct TouchArea {
  int16_t x, y, w, h;
  TouchButton id;
};

// ============================================
// TEA5767 DRIVER
// ============================================
#define TEA5767_ADDR        0x60
#define TEA5767_FM_LOW      76.0
#define TEA5767_FM_HIGH     108.0
#define TEA5767_DEFAULT_FREQ 101.1

struct TEA5767_Status {
  float frequency;
  bool stereo;
  uint8_t signalLevel;
  bool bandLimitReached;
  bool ready;
};

class TEA5767Radio {
public:
  float frequency;
  bool muted, standby, stereoMode;
  uint8_t searchStopLevel;
  uint8_t writeData[5], readData[5];
  static const int MAX_PRESETS = 30;
  float presets[MAX_PRESETS];
  int presetCount, currentPreset;

  TEA5767Radio() : frequency(TEA5767_DEFAULT_FREQ), muted(false), standby(false),
    stereoMode(true), searchStopLevel(2), presetCount(0), currentPreset(-1) {
    memset(writeData, 0, 5);
    memset(readData, 0, 5);
    float def[] = {89.1, 91.3, 93.7, 96.1, 98.3, 100.9, 101.1, 103.3, 105.1, 107.5};
    presetCount = 10;
    for (int i = 0; i < presetCount; i++) presets[i] = def[i];
  }

  bool begin(int sda, int scl) {
    Wire.begin(sda, scl);
    Wire.beginTransmission(TEA5767_ADDR);
    if (Wire.endTransmission() != 0) return false;
    setFrequency(frequency);
    return true;
  }

  uint16_t calcPLL(float f) { return (uint16_t)(4.0 * (f * 1e6 + 225000.0) / 32768.0); }
  float calcFreq(uint16_t pll) { return ((pll * 32768.0) / 4.0 - 225000.0) / 1e6; }

  void transmit() {
    uint16_t pll = calcPLL(frequency);
    writeData[0] = (pll >> 8) & 0x3F;
    if (muted) writeData[0] |= 0x80;
    writeData[1] = pll & 0xFF;
    writeData[2] = 0x10 | ((searchStopLevel & 0x03) << 5);
    writeData[3] = 0x10;
    if (!stereoMode) writeData[3] |= 0x08;
    if (standby) writeData[3] |= 0x40;
    writeData[4] = 0x00;
    Wire.beginTransmission(TEA5767_ADDR);
    Wire.write(writeData, 5);
    Wire.endTransmission();
  }

  void readStatus() {
    Wire.requestFrom((uint8_t)TEA5767_ADDR, (uint8_t)5);
    for (int i = 0; i < 5 && Wire.available(); i++) readData[i] = Wire.read();
  }

  void setFrequency(float f) {
    frequency = constrain(f, TEA5767_FM_LOW, TEA5767_FM_HIGH);
    transmit();
  }

  void seekUp() {
    uint16_t pll = calcPLL(frequency);
    writeData[0] = ((pll >> 8) & 0x3F) | 0x40;
    if (muted) writeData[0] |= 0x80;
    writeData[1] = pll & 0xFF;
    writeData[2] = 0xB0 | 0x80;
    writeData[3] = 0x10;
    writeData[4] = 0x00;
    Wire.beginTransmission(TEA5767_ADDR); Wire.write(writeData, 5); Wire.endTransmission();
    delay(500); readStatus();
    frequency = calcFreq((uint16_t)(((readData[0] & 0x3F) << 8) | readData[1]));
  }

  void seekDown() {
    uint16_t pll = calcPLL(frequency);
    writeData[0] = ((pll >> 8) & 0x3F) | 0x40;
    if (muted) writeData[0] |= 0x80;
    writeData[1] = pll & 0xFF;
    writeData[2] = 0x30;
    writeData[3] = 0x10;
    writeData[4] = 0x00;
    Wire.beginTransmission(TEA5767_ADDR); Wire.write(writeData, 5); Wire.endTransmission();
    delay(500); readStatus();
    frequency = calcFreq((uint16_t)(((readData[0] & 0x3F) << 8) | readData[1]));
  }

  void setMute(bool m) { muted = m; transmit(); }
  void setStandby(bool s) { standby = s; transmit(); }

  TEA5767_Status getStatus() {
    readStatus();
    TEA5767_Status st;
    uint16_t pll = ((readData[0] & 0x3F) << 8) | readData[1];
    st.frequency = calcFreq(pll);
    st.ready = (readData[0] & 0x80) != 0;
    st.bandLimitReached = (readData[0] & 0x40) != 0;
    st.stereo = (readData[2] & 0x80) != 0;
    st.signalLevel = (readData[3] >> 4) & 0x0F;
    return st;
  }

  void nextPreset() {
    if (presetCount == 0) return;
    currentPreset = (currentPreset + 1) % presetCount;
    setFrequency(presets[currentPreset]);
  }

  void prevPreset() {
    if (presetCount == 0) return;
    currentPreset--;
    if (currentPreset < 0) currentPreset = presetCount - 1;
    setFrequency(presets[currentPreset]);
  }
};

// ============================================
// BLUETOOTH A2DP AUDIO
// ============================================
class BTAudio {
public:
  BluetoothA2DPSink a2dp;
  bool connected, playing;
  uint8_t vol;
  char deviceName[64];
  typedef void (*ConnCb)(bool);
  typedef void (*PlayCb)(bool);
  ConnCb connCb;
  PlayCb playCb;
  static BTAudio* inst;

  BTAudio() : connected(false), playing(false), vol(VOLUME_DEFAULT),
    connCb(nullptr), playCb(nullptr) {
    memset(deviceName, 0, 64);
    inst = this;
  }

  void begin(const char* name) {
    i2s_pin_config_t pins = {
      .bck_io_num = I2S_BCLK, .ws_io_num = I2S_LRC,
      .data_out_num = I2S_DOUT, .data_in_num = I2S_PIN_NO_CHANGE
    };
    a2dp.set_pin_config(pins);
    a2dp.set_on_connection_state_changed(onConn);
    a2dp.set_on_audio_state_changed(onAudio);
    a2dp.set_volume(vol * 127 / 100);
    a2dp.start(name);
  }

  void end() { a2dp.end(true); connected = false; playing = false; }
  void setVolume(uint8_t v) { vol = min(v, (uint8_t)VOLUME_MAX); a2dp.set_volume(vol * 127 / 100); }
  void play() { a2dp.play(); }
  void pause() { a2dp.pause(); }

  static void onConn(esp_a2d_connection_state_t state, void*) {
    if (!inst) return;
    if (state == ESP_A2D_CONNECTION_STATE_CONNECTED) {
      inst->connected = true;
      strncpy(inst->deviceName, "Alexa", 63);
      if (inst->connCb) inst->connCb(true);
    } else if (state == ESP_A2D_CONNECTION_STATE_DISCONNECTED) {
      inst->connected = false; inst->playing = false;
      memset(inst->deviceName, 0, 64);
      if (inst->connCb) inst->connCb(false);
    }
  }

  static void onAudio(esp_a2d_audio_state_t state, void*) {
    if (!inst) return;
    if (state == ESP_A2D_AUDIO_STATE_STARTED) {
      inst->playing = true;
      if (inst->playCb) inst->playCb(true);
    } else if (state == ESP_A2D_AUDIO_STATE_REMOTE_SUSPEND || state == ESP_A2D_AUDIO_STATE_STOPPED) {
      inst->playing = false;
      if (inst->playCb) inst->playCb(false);
    }
  }
};
BTAudio* BTAudio::inst = nullptr;

// ============================================
// LED EFFECTS (WS2812B)
// ============================================
class LEDEffects {
public:
  CRGB leds[LED_COUNT];
  LEDEffect effect;
  uint8_t brightness, audioSmooth, peak, peakDecay, hue;
  unsigned long lastUpdate;

  LEDEffects() : effect(LED_VU_METER), brightness(LED_BRIGHTNESS),
    audioSmooth(0), peak(0), peakDecay(0), hue(0), lastUpdate(0) {}

  void begin() {
    FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, LED_COUNT);
    FastLED.setBrightness(brightness);
    FastLED.clear(); FastLED.show();
    analogReadResolution(12);
    analogSetAttenuation(ADC_11db);
    pinMode(AUDIO_IN_PIN, INPUT);
  }

  void readAudio() {
    uint16_t maxVal = 0;
    for (int i = 0; i < AUDIO_SAMPLES; i++) {
      int16_t s = (int16_t)analogRead(AUDIO_IN_PIN) - 2048;
      if (s < 0) s = -s;
      if (s > maxVal) maxVal = s;
    }
    uint8_t raw = map(constrain(maxVal, AUDIO_NOISE_FLOOR, 2048), AUDIO_NOISE_FLOOR, 2048, 0, 255);
    audioSmooth = (audioSmooth * 3 + raw) / 4;
    if (audioSmooth >= peak) { peak = audioSmooth; peakDecay = 20; }
    else { if (peakDecay > 0) peakDecay--; else if (peak > 2) peak -= 2; else peak = 0; }
  }

  void update() {
    if (millis() - lastUpdate < LED_UPDATE_MS) return;
    lastUpdate = millis();
    readAudio(); hue++;
    switch (effect) {
      case LED_OFF: FastLED.clear(); break;
      case LED_VU_METER: vuMeter(); break;
      case LED_SPECTRUM: spectrum(); break;
      case LED_PULSE: pulse(); break;
      case LED_RAINBOW: rainbow(); break;
      case LED_FIRE: fire(); break;
      default: FastLED.clear(); break;
    }
    FastLED.show();
  }

  void vuMeter() {
    int lit = map(audioSmooth, 0, 255, 0, LED_COUNT);
    int pk = map(peak, 0, 255, 0, LED_COUNT - 1);
    for (int i = 0; i < LED_COUNT; i++) {
      if (i < lit) {
        if (i < LED_COUNT * 6 / 10) leds[i] = CRGB::Green;
        else if (i < LED_COUNT * 8 / 10) leds[i] = CRGB::Yellow;
        else leds[i] = CRGB::Red;
      } else leds[i] = CRGB::Black;
    }
    if (pk > 0 && pk < LED_COUNT) leds[pk] = CRGB::White;
  }

  void spectrum() {
    int lit = map(audioSmooth, 0, 255, 1, LED_COUNT);
    for (int i = 0; i < LED_COUNT; i++) {
      if (i < lit) leds[i] = CHSV(hue + (i * 255 / LED_COUNT), 255, map(i, 0, lit, 255, 100));
      else leds[i].fadeToBlackBy(40);
    }
  }

  void pulse() {
    static uint8_t lastS = 0, pVal = 0, pHue = 0;
    if (audioSmooth > lastS + 30) { pVal = 255; pHue += 32; }
    lastS = audioSmooth;
    if (pVal > 4) pVal -= 4; else pVal = 0;
    uint8_t amb = audioSmooth / 6;
    for (int i = 0; i < LED_COUNT; i++) leds[i] = CHSV(pHue, 255, qadd8(pVal, amb));
  }

  void rainbow() {
    uint8_t intensity = map(audioSmooth, 0, 255, 40, 255);
    uint8_t spd = map(audioSmooth, 0, 255, 1, 8);
    fill_rainbow(leds, LED_COUNT, hue * spd, 255 / LED_COUNT);
    for (int i = 0; i < LED_COUNT; i++) leds[i].nscale8(intensity);
  }

  void fire() {
    static uint8_t heat[LED_COUNT];
    uint8_t cooling = map(audioSmooth, 0, 255, 20, 80);
    uint8_t sparking = map(audioSmooth, 0, 255, 50, 240);
    for (int i = 0; i < LED_COUNT; i++)
      heat[i] = qsub8(heat[i], random8(0, ((cooling * 10) / LED_COUNT) + 2));
    for (int k = LED_COUNT - 1; k >= 2; k--)
      heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
    if (random8() < sparking) { int y = random8(7); heat[y] = qadd8(heat[y], random8(160, 255)); }
    for (int j = 0; j < LED_COUNT; j++) {
      uint8_t t = scale8_video(heat[j], 191);
      uint8_t hr = (t & 0x3F) << 2;
      if (t & 0x80) leds[j] = CRGB(255, 255, hr);
      else if (t & 0x40) leds[j] = CRGB(255, hr, 0);
      else leds[j] = CRGB(hr, 0, 0);
    }
  }

  void nextEffect() {
    effect = (LEDEffect)(((int)effect + 1) % LED_EFFECT_COUNT);
    FastLED.clear(); FastLED.show();
  }

  void cycleBrightness() {
    brightness += 50;
    if (brightness > 250) brightness = 50;
    FastLED.setBrightness(brightness);
  }

  const char* getEffectName() {
    switch (effect) {
      case LED_OFF: return "OFF";
      case LED_VU_METER: return "VU METER";
      case LED_SPECTRUM: return "SPECTRUM";
      case LED_PULSE: return "PULSE";
      case LED_RAINBOW: return "RAINBOW";
      case LED_FIRE: return "FIRE";
      default: return "---";
    }
  }
};

// ============================================
// DISPLAY UI PIONEER
// ============================================
#define MAX_BUTTONS 12

class DisplayUI {
public:
  TFT_eSPI tft;
  TFT_eSprite sprFreq;
  TFT_eSprite sprEQ;
  RadioMode curMode;
  float lastFreq; uint8_t lastSig; bool lastStereo, lastMuted; uint8_t lastVol;
  unsigned long lastTouch, lastEQ;
  uint8_t eqBars[EQ_BARS], eqPeaks[EQ_BARS], eqDelay[EQ_BARS];
  TouchArea buttons[MAX_BUTTONS];
  int btnCount;

  DisplayUI() : tft(TFT_eSPI()), sprFreq(&tft), sprEQ(&tft),
    curMode(MODE_FM), lastFreq(0), lastSig(0), lastStereo(false),
    lastMuted(false), lastVol(0), lastTouch(0), lastEQ(0), btnCount(0) {
    memset(eqBars, 0, EQ_BARS); memset(eqPeaks, 0, EQ_BARS); memset(eqDelay, 0, EQ_BARS);
  }

  void begin() {
    tft.init(); tft.setRotation(0); tft.fillScreen(COLOR_BG);
    sprFreq.createSprite(224, 55);
    sprEQ.createSprite(224, EQ_MAX_HEIGHT + 4);
    uint16_t cal[5] = {TOUCH_MIN_X, TOUCH_MAX_X, TOUCH_MIN_Y, TOUCH_MAX_Y, 1};
    tft.setTouch(cal);
  }

  void drawGlowLine(int y, uint16_t c) {
    tft.drawFastHLine(8, y - 1, SCREEN_WIDTH - 16, COLOR_FREQ_GLOW);
    tft.drawFastHLine(4, y, SCREEN_WIDTH - 8, c);
    tft.drawFastHLine(8, y + 1, SCREEN_WIDTH - 16, COLOR_FREQ_GLOW);
  }

  void drawSplashScreen() {
    tft.fillScreen(COLOR_BG);
    for (int i = 0; i < 3; i++)
      tft.drawFastHLine(20 + i * 5, 40 + i * 2, 200 - i * 10, COLOR_FREQ_GLOW);
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(COLOR_FREQ_GLOW); tft.drawString("ESP32", 121, 101, 4); tft.drawString("RADIO", 121, 141, 4);
    tft.setTextColor(COLOR_NEON_CYAN); tft.drawString("ESP32", 120, 100, 4); tft.drawString("RADIO", 120, 140, 4);
    tft.setTextColor(COLOR_NEON_BLUE); tft.drawString("FM + BLUETOOTH", 120, 180, 2);
    drawGlowLine(200, COLOR_NEON_CYAN);
    tft.setTextColor(COLOR_TEXT_DIM); tft.drawString("TOUCH CONTROL", 120, 220, 2);
    int barY = 275;
    tft.drawRoundRect(29, barY - 1, 182, 12, 4, COLOR_FREQ_GLOW);
    for (int i = 0; i <= 100; i += 2) {
      int w = (i * 178) / 100;
      tft.fillRoundRect(31, barY + 1, w, 8, 3, (i < 50) ? COLOR_NEON_GREEN : COLOR_NEON_CYAN);
      delay(15);
    }
    delay(400);
  }

  void drawHeader(RadioMode mode) {
    tft.fillRect(0, 0, SCREEN_WIDTH, 33, COLOR_HEADER_BG);
    tft.drawFastHLine(0, 0, SCREEN_WIDTH, COLOR_BTN_BORDER);
    tft.setTextDatum(ML_DATUM);
    tft.setTextColor(COLOR_NEON_CYAN, COLOR_HEADER_BG); tft.drawString("ESP32", 6, 16, 2);
    tft.setTextColor(COLOR_TEXT_DIM, COLOR_HEADER_BG); tft.drawString("RADIO", 52, 16, 2);
    tft.setTextDatum(MC_DATUM);
    if (mode == MODE_FM) {
      tft.fillRoundRect(176, 5, 56, 22, 4, COLOR_BTN_MODE_FM);
      tft.drawRoundRect(176, 5, 56, 22, 4, COLOR_NEON_GREEN);
      tft.setTextColor(COLOR_BG, COLOR_BTN_MODE_FM); tft.drawString("FM", 204, 16, 2);
    } else {
      tft.fillRoundRect(170, 5, 64, 22, 4, COLOR_BTN_MODE_BT);
      tft.drawRoundRect(170, 5, 64, 22, 4, COLOR_NEON_BLUE);
      tft.setTextColor(COLOR_NEON_WHITE, COLOR_BTN_MODE_BT); tft.drawString("BT", 202, 16, 2);
    }
    tft.drawFastHLine(0, 32, SCREEN_WIDTH, COLOR_BTN_BORDER);
  }

  void drawSignalMeter(uint8_t level) {
    int x = 160, y = 40;
    tft.fillRect(x, y, 76, 14, COLOR_BG);
    tft.setTextDatum(ML_DATUM);
    tft.setTextColor(COLOR_TEXT_DIM, COLOR_BG); tft.drawString("SIG", x, y + 7, 1);
    int bx = x + 22;
    for (int i = 0; i < 5; i++) {
      uint16_t c = (i < (int)(level * 5 / 15)) ?
        ((i < 2) ? COLOR_NEON_GREEN : (i < 4) ? COLOR_NEON_CYAN : COLOR_NEON_BLUE) : COLOR_EQ_BG;
      tft.fillRect(bx + i * 11, y + 2, 9, 10, c);
    }
  }

  void drawEQ() {
    sprEQ.fillSprite(COLOR_BG);
    int bW = 12, gap = 2, sx = (224 - EQ_BARS * (bW + gap)) / 2;
    for (int i = 0; i < EQ_BARS; i++) {
      int x = sx + i * (bW + gap), h = eqBars[i], y = EQ_MAX_HEIGHT - h;
      for (int r = 0; r < h; r++) {
        float ratio = (float)(h - r) / EQ_MAX_HEIGHT;
        sprEQ.drawFastHLine(x, y + r, bW,
          (ratio < 0.33) ? COLOR_EQ_LOW : (ratio < 0.66) ? COLOR_EQ_MID : COLOR_EQ_HIGH);
      }
      if (eqPeaks[i] > 0) sprEQ.drawFastHLine(x, EQ_MAX_HEIGHT - eqPeaks[i], bW, COLOR_EQ_PEAK);
    }
    sprEQ.pushSprite(8, 142);
  }

  void updateEQ() {
    if (millis() - lastEQ < EQ_UPDATE_MS) return;
    lastEQ = millis();
    for (int i = 0; i < EQ_BARS; i++) {
      int t = random(3, EQ_MAX_HEIGHT - 2);
      if (eqBars[i] < t) eqBars[i] += random(1, 4);
      else if (eqBars[i] > t) eqBars[i] -= random(1, 3);
      if (eqBars[i] > EQ_MAX_HEIGHT) eqBars[i] = EQ_MAX_HEIGHT;
      if (eqBars[i] >= eqPeaks[i]) { eqPeaks[i] = eqBars[i]; eqDelay[i] = 6; }
      else { if (eqDelay[i] > 0) eqDelay[i]--; else if (eqPeaks[i] > 0) eqPeaks[i]--; }
    }
    drawEQ();
  }

  void drawBtn(int16_t x, int16_t y, int16_t w, int16_t h, const char* label, uint16_t glow, bool active = false) {
    uint16_t bg = active ? COLOR_BTN_PRESSED : COLOR_BTN_BG;
    tft.fillRoundRect(x + 1, y + 1, w - 2, h - 2, 5, bg);
    tft.drawRoundRect(x, y, w, h, 5, glow);
    tft.drawFastHLine(x + 4, y + 1, w - 8, COLOR_DIVIDER);
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(glow, bg);
    tft.drawString(label, x + w / 2, y + h / 2, 2);
  }

  void clearBtns() { btnCount = 0; }
  void regBtn(int16_t x, int16_t y, int16_t w, int16_t h, TouchButton id) {
    if (btnCount >= MAX_BUTTONS) return;
    buttons[btnCount++] = {x, y, w, h, id};
  }

  TouchButton checkTouch() {
    if (millis() - lastTouch < TOUCH_DEBOUNCE_MS) return BTN_NONE;
    uint16_t tx, ty;
    if (!tft.getTouch(&tx, &ty)) return BTN_NONE;
    for (int i = 0; i < btnCount; i++) {
      TouchArea &b = buttons[i];
      if ((int16_t)tx >= b.x && (int16_t)tx <= b.x + b.w && (int16_t)ty >= b.y && (int16_t)ty <= b.y + b.h) {
        lastTouch = millis();
        return b.id;
      }
    }
    return BTN_NONE;
  }

  void updateFrequency(float f) {
    sprFreq.fillSprite(COLOR_BG);
    sprFreq.setTextDatum(MC_DATUM);
    char s[10]; snprintf(s, 10, "%.1f", f);
    sprFreq.setTextColor(COLOR_FREQ_GLOW); sprFreq.drawString(s, 113, 28, 7);
    sprFreq.setTextColor(COLOR_FREQ); sprFreq.drawString(s, 112, 27, 7);
    sprFreq.pushSprite(8, 65);
    lastFreq = f;
  }

  void updateStereo(bool st) {
    tft.fillRect(4, 38, 100, 16, COLOR_BG);
    tft.setTextDatum(ML_DATUM);
    if (st) { tft.setTextColor(COLOR_STEREO, COLOR_BG); tft.drawString("STEREO", 6, 46, 2); }
    else { tft.setTextColor(COLOR_TEXT_DIM, COLOR_BG); tft.drawString("MONO", 6, 46, 2); }
    lastStereo = st;
  }

  void updateMute(bool m) {
    lastMuted = m;
    int y = (curMode == MODE_FM) ? 233 : 231;
    if (m) drawBtn(74, y, 92, 30, "MUDO", COLOR_NEON_RED, true);
    else drawBtn(74, y, 92, 30, "MUTE", COLOR_NEON_RED);
  }

  void updateVolume(uint8_t v) {
    if (v == lastVol) return;
    int bx = 4, by = 313, bw = SCREEN_WIDTH - 40;
    tft.fillRoundRect(bx, by, bw, 6, 3, COLOR_VOL_BG);
    int fw = (v * bw) / VOLUME_MAX;
    uint16_t fc = (v > 95) ? COLOR_VOL_MAX : (v > 80) ? COLOR_VOL_HIGH : COLOR_VOL_BAR;
    if (fw > 0) { tft.fillRoundRect(bx, by, fw, 6, 3, fc); tft.fillCircle(bx + fw, by + 3, 4, fc); }
    char vs[6]; snprintf(vs, 6, "%d", v);
    tft.fillRect(bx + bw + 2, by - 4, 36, 14, COLOR_BG);
    tft.setTextDatum(ML_DATUM); tft.setTextColor(fc, COLOR_BG); tft.drawString(vs, bx + bw + 6, by + 2, 2);
    lastVol = v;
  }

  void updatePreset(int idx, float freq, int total) {
    tft.fillRect(0, 178, SCREEN_WIDTH, 16, COLOR_BG);
    tft.setTextDatum(MC_DATUM);
    if (total <= 0 || idx < 0) {
      tft.setTextColor(COLOR_TEXT_DIM, COLOR_BG); tft.drawString("SEM PRESET", 120, 186, 1);
    } else {
      char s[32]; snprintf(s, 32, "P%d/%d  %.1f MHz", idx + 1, total, freq);
      tft.setTextColor(COLOR_NEON_ORANGE, COLOR_BG); tft.drawString(s, 120, 186, 2);
    }
  }

  void updateBTStatus(bool conn, const char* name) {
    tft.fillRect(0, 112, SCREEN_WIDTH, 28, COLOR_BG);
    tft.setTextDatum(MC_DATUM);
    if (conn) {
      tft.setTextColor(COLOR_BT_CONNECTED, COLOR_BG); tft.drawString("CONECTADO", 120, 118, 2);
      tft.setTextColor(COLOR_TEXT, COLOR_BG); tft.drawString(name ? name : "Alexa", 120, 134, 2);
    } else {
      tft.setTextColor(COLOR_BT_WAITING, COLOR_BG); tft.drawString("AGUARDANDO...", 120, 118, 2);
      tft.setTextColor(COLOR_TEXT_DIM, COLOR_BG); tft.drawString(BT_DEVICE_NAME, 120, 134, 1);
    }
  }

  void updatePlay(bool p) {
    tft.fillRect(40, 136, 160, 16, COLOR_BG);
    tft.setTextDatum(MC_DATUM);
    if (p) { tft.setTextColor(COLOR_NEON_GREEN, COLOR_BG); tft.drawString(">> TOCANDO >>", 120, 137, 1); }
    else { tft.setTextColor(COLOR_TEXT_DIM, COLOR_BG); tft.drawString("PAUSADO", 120, 137, 1); }
  }

  void showSeeking() {
    sprFreq.fillSprite(COLOR_BG); sprFreq.setTextDatum(MC_DATUM);
    sprFreq.setTextColor(COLOR_NEON_ORANGE); sprFreq.drawString("SEEK...", 112, 27, 4);
    sprFreq.pushSprite(8, 65);
  }

  void showMsg(const char* msg, uint16_t c = COLOR_TEXT) {
    tft.fillRect(0, 178, SCREEN_WIDTH, 16, COLOR_BG);
    tft.setTextDatum(MC_DATUM); tft.setTextColor(c, COLOR_BG); tft.drawString(msg, 120, 186, 2);
  }

  void drawFMScreen(float freq, bool stereo, uint8_t sig, bool muted, int pi, int pc, uint8_t vol, const char* led) {
    tft.fillScreen(COLOR_BG); curMode = MODE_FM; clearBtns();
    drawHeader(MODE_FM); updateStereo(stereo); drawSignalMeter(sig);
    tft.drawFastHLine(4, 56, SCREEN_WIDTH - 8, COLOR_DIVIDER);
    updateFrequency(freq);
    tft.setTextDatum(MC_DATUM); tft.setTextColor(COLOR_TEXT_DIM, COLOR_BG); tft.drawString("FM MHz", 120, 130, 2);
    for (int i = 0; i < EQ_BARS; i++) { eqBars[i] = random(5, EQ_MAX_HEIGHT); eqPeaks[i] = eqBars[i]; eqDelay[i] = 3; }
    drawEQ();
    tft.drawFastHLine(4, 176, SCREEN_WIDTH - 8, COLOR_DIVIDER);
    float pf = (pi >= 0 && pi < pc) ? freq : 0;
    updatePreset(pi, pf, pc);
    updateVolume(vol);
    // Touch buttons
    int r1 = 197, r2 = 233, r3 = 265, r4 = 293, h = 30;
    drawBtn(4, r1, 68, h, "<< SEEK", COLOR_NEON_CYAN); regBtn(4, r1, 68, h, BTN_SEEK_DOWN);
    drawBtn(76, r1, 42, h, "<P", COLOR_NEON_ORANGE); regBtn(76, r1, 42, h, BTN_PRESET_PREV);
    drawBtn(122, r1, 42, h, "P>", COLOR_NEON_ORANGE); regBtn(122, r1, 42, h, BTN_PRESET_NEXT);
    drawBtn(168, r1, 68, h, "SEEK >>", COLOR_NEON_CYAN); regBtn(168, r1, 68, h, BTN_SEEK_UP);
    drawBtn(4, r2, 66, h, "VOL -", COLOR_NEON_GREEN); regBtn(4, r2, 66, h, BTN_VOL_DOWN);
    if (muted) drawBtn(74, r2, 92, h, "MUDO", COLOR_NEON_RED, true);
    else drawBtn(74, r2, 92, h, "MUTE", COLOR_NEON_RED);
    regBtn(74, r2, 92, h, BTN_MUTE);
    drawBtn(170, r2, 66, h, "VOL +", COLOR_NEON_GREEN); regBtn(170, r2, 66, h, BTN_VOL_UP);
    drawBtn(4, r3, 136, h - 4, led, COLOR_NEON_MAGENTA); regBtn(4, r3, 136, h - 4, BTN_LED_MODE);
    drawBtn(144, r3, 92, h - 4, "BRILHO", COLOR_NEON_MAGENTA); regBtn(144, r3, 92, h - 4, BTN_LED_BRIGHT);
    drawBtn(4, r4, SCREEN_WIDTH - 8, h - 4, "BLUETOOTH", COLOR_NEON_BLUE); regBtn(4, r4, SCREEN_WIDTH - 8, h - 4, BTN_MODE);
    lastFreq = freq; lastSig = sig; lastStereo = stereo; lastMuted = muted; lastVol = vol;
  }

  void drawBTScreen(bool conn, const char* name, bool muted, uint8_t vol, bool playing, const char* led) {
    tft.fillScreen(COLOR_BG); curMode = MODE_BLUETOOTH; clearBtns();
    drawHeader(MODE_BLUETOOTH);
    tft.fillCircle(120, 78, 30, COLOR_HEADER_BG);
    tft.drawCircle(120, 78, 30, COLOR_BTN_BORDER);
    tft.drawCircle(120, 78, 32, COLOR_FREQ_GLOW);
    tft.setTextDatum(MC_DATUM); tft.setTextColor(COLOR_BT_ICON, COLOR_HEADER_BG); tft.drawString("BT", 120, 78, 4);
    updateBTStatus(conn, name); updatePlay(playing);
    for (int i = 0; i < EQ_BARS; i++) { eqBars[i] = playing ? random(5, EQ_MAX_HEIGHT) : random(1, 5); eqPeaks[i] = eqBars[i]; eqDelay[i] = 3; }
    drawEQ();
    tft.drawFastHLine(4, 176, SCREEN_WIDTH - 8, COLOR_DIVIDER);
    updateVolume(vol);
    int r1 = 197, r2 = 231, r3 = 261, r4 = 293, h = 30;
    if (playing) drawBtn(4, r1, SCREEN_WIDTH - 8, h, "|| PAUSAR", COLOR_NEON_ORANGE, true);
    else drawBtn(4, r1, SCREEN_WIDTH - 8, h, "> TOCAR", COLOR_NEON_GREEN);
    regBtn(4, r1, SCREEN_WIDTH - 8, h, BTN_PLAY_PAUSE);
    drawBtn(4, r2, 66, h, "VOL -", COLOR_NEON_GREEN); regBtn(4, r2, 66, h, BTN_VOL_DOWN);
    if (muted) drawBtn(74, r2, 92, h, "MUDO", COLOR_NEON_RED, true);
    else drawBtn(74, r2, 92, h, "MUTE", COLOR_NEON_RED);
    regBtn(74, r2, 92, h, BTN_MUTE);
    drawBtn(170, r2, 66, h, "VOL +", COLOR_NEON_GREEN); regBtn(170, r2, 66, h, BTN_VOL_UP);
    drawBtn(4, r3, 136, h - 4, led, COLOR_NEON_MAGENTA); regBtn(4, r3, 136, h - 4, BTN_LED_MODE);
    drawBtn(144, r3, 92, h - 4, "BRILHO", COLOR_NEON_MAGENTA); regBtn(144, r3, 92, h - 4, BTN_LED_BRIGHT);
    drawBtn(4, r4, SCREEN_WIDTH - 8, h - 4, "FM RADIO", COLOR_NEON_GREEN); regBtn(4, r4, SCREEN_WIDTH - 8, h - 4, BTN_MODE);
    lastMuted = muted; lastVol = vol;
  }
};

// ============================================
// OBJETOS GLOBAIS
// ============================================
TEA5767Radio radio;
DisplayUI   display;
BTAudio     btAudio;
LEDEffects  ledStrip;

RadioMode   currentMode = MODE_FM;
uint8_t     volume = VOLUME_DEFAULT;
bool        isMuted = false;
bool        btStarted = false;
unsigned long lastStatusRead = 0;

// ============================================
// CALLBACKS BT
// ============================================
void onBTConn(bool c) { if (currentMode == MODE_BLUETOOTH) display.updateBTStatus(c, btAudio.deviceName); }
void onBTPlay(bool p) { if (currentMode == MODE_BLUETOOTH) display.updatePlay(p); }

// ============================================
// FUNÇÕES
// ============================================
void switchToFM() {
  currentMode = MODE_FM;
  if (btStarted) { btAudio.end(); btStarted = false; delay(500); }
  radio.setStandby(false);
  TEA5767_Status st = radio.getStatus();
  display.drawFMScreen(radio.frequency, st.stereo, st.signalLevel, isMuted,
    radio.currentPreset, radio.presetCount, volume, ledStrip.getEffectName());
}

void switchToBT() {
  currentMode = MODE_BLUETOOTH;
  radio.setStandby(true);
  if (!btStarted) {
    btAudio.begin(BT_DEVICE_NAME);
    btAudio.connCb = onBTConn;
    btAudio.playCb = onBTPlay;
    btAudio.setVolume(volume);
    btStarted = true;
  }
  display.drawBTScreen(btAudio.connected, btAudio.deviceName, isMuted,
    volume, btAudio.playing, ledStrip.getEffectName());
}

void adjustVol(int d) {
  int v = (int)volume + d;
  volume = (uint8_t)constrain(v, 0, VOLUME_MAX);
  if (currentMode == MODE_BLUETOOTH && btStarted) btAudio.setVolume(volume);
  display.updateVolume(volume);
}

void toggleMute() {
  isMuted = !isMuted;
  if (currentMode == MODE_FM) radio.setMute(isMuted);
  display.updateMute(isMuted);
}

void handleTouch() {
  TouchButton btn = display.checkTouch();
  if (btn == BTN_NONE) return;
  switch (btn) {
    case BTN_MODE: if (currentMode == MODE_FM) switchToBT(); else switchToFM(); break;
    case BTN_SEEK_UP:
      if (currentMode == MODE_FM) {
        display.showSeeking(); radio.seekUp();
        TEA5767_Status s = radio.getStatus();
        display.updateFrequency(radio.frequency);
        display.drawSignalMeter(s.signalLevel); display.updateStereo(s.stereo);
      } break;
    case BTN_SEEK_DOWN:
      if (currentMode == MODE_FM) {
        display.showSeeking(); radio.seekDown();
        TEA5767_Status s = radio.getStatus();
        display.updateFrequency(radio.frequency);
        display.drawSignalMeter(s.signalLevel); display.updateStereo(s.stereo);
      } break;
    case BTN_PRESET_NEXT:
      if (currentMode == MODE_FM) {
        radio.nextPreset(); display.updateFrequency(radio.frequency);
        display.updatePreset(radio.currentPreset, radio.frequency, radio.presetCount);
      } break;
    case BTN_PRESET_PREV:
      if (currentMode == MODE_FM) {
        radio.prevPreset(); display.updateFrequency(radio.frequency);
        display.updatePreset(radio.currentPreset, radio.frequency, radio.presetCount);
      } break;
    case BTN_VOL_UP: adjustVol(VOLUME_STEP); break;
    case BTN_VOL_DOWN: adjustVol(-VOLUME_STEP); break;
    case BTN_MUTE: toggleMute(); break;
    case BTN_PLAY_PAUSE:
      if (currentMode == MODE_BLUETOOTH && btStarted) {
        if (btAudio.playing) btAudio.pause(); else btAudio.play();
      } break;
    case BTN_LED_MODE:
      ledStrip.nextEffect();
      display.showMsg(ledStrip.getEffectName(), COLOR_NEON_MAGENTA); break;
    case BTN_LED_BRIGHT:
      ledStrip.cycleBrightness();
      { char m[20]; snprintf(m, 20, "LED: %d%%", ledStrip.brightness * 100 / 255);
        display.showMsg(m, COLOR_NEON_MAGENTA); } break;
    default: break;
  }
}

// ============================================
// SETUP
// ============================================
void setup() {
  Serial.begin(115200);
  Serial.println("\n=== ESP32 Radio - Pioneer Style ===");

  display.begin();
  display.drawSplashScreen();

  bool ok = radio.begin(I2C_SDA, I2C_SCL);
  if (!ok) { display.showMsg("TEA5767 nao encontrado!", COLOR_NEON_RED); delay(2000); }

  ledStrip.begin();
  switchToFM();

  Serial.println("[INIT] Pronto!");
}

// ============================================
// LOOP
// ============================================
void loop() {
  handleTouch();
  display.updateEQ();
  ledStrip.update();

  if (millis() - lastStatusRead >= STATUS_READ_MS) {
    lastStatusRead = millis();
    if (currentMode == MODE_FM) {
      TEA5767_Status st = radio.getStatus();
      display.drawSignalMeter(st.signalLevel);
      display.updateStereo(st.stereo);
    }
  }
  delay(5);
}
