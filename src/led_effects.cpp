/**
 * ESP32 Radio Bluetooth - WS2812B LED Effects
 * LED strip reativa ao som com múltiplos efeitos
 *
 * Conecte a saída de áudio (FM ou BT) a um divisor de tensão
 * e ligue ao pino ADC (GPIO 36/VP) para captura de nível de áudio.
 *
 * Circuito sugerido:
 *   Audio Out ──[10kΩ]──┬──[10kΩ]──GND
 *                       │
 *                       └── GPIO 36 (VP)
 *                       │
 *                      [100nF] capacitor para GND
 */

#include "led_effects.h"

LEDEffects::LEDEffects()
    : _currentEffect(LED_VU_METER),
      _brightness(LED_BRIGHTNESS),
      _audioLevel(0),
      _audioSmooth(0),
      _peak(0),
      _peakDecay(0),
      _hue(0),
      _lastUpdate(0)
{
}

void LEDEffects::begin() {
    FastLED.addLeds<LED_TYPE, LED_PIN, LED_COLOR_ORDER>(_leds, LED_COUNT);
    FastLED.setBrightness(_brightness);
    FastLED.clear();
    FastLED.show();

    analogReadResolution(12);
    analogSetAttenuation(ADC_11db);
    pinMode(AUDIO_IN_PIN, INPUT);

    Serial.printf("[LED] WS2812B iniciado: %d LEDs no GPIO %d\n", LED_COUNT, LED_PIN);
}

void LEDEffects::update() {
    unsigned long now = millis();
    if (now - _lastUpdate < LED_UPDATE_MS) return;
    _lastUpdate = now;

    readAudio();
    _hue++;

    switch (_currentEffect) {
    case LED_OFF:       effectOff(); break;
    case LED_VU_METER:  effectVUMeter(); break;
    case LED_SPECTRUM:  effectSpectrum(); break;
    case LED_PULSE:     effectPulse(); break;
    case LED_RAINBOW:   effectRainbow(); break;
    case LED_FIRE:      effectFire(); break;
    default:            effectOff(); break;
    }

    FastLED.show();
}

// ============================================
// Audio Processing
// ============================================
void LEDEffects::readAudio() {
    uint32_t sum = 0;
    uint16_t maxVal = 0;

    for (int i = 0; i < AUDIO_SAMPLES; i++) {
        uint16_t sample = analogRead(AUDIO_IN_PIN);
        // Center around midpoint (1024 for 12-bit with bias at ~0.8V)
        int16_t centered = (int16_t)sample - 2048;
        if (centered < 0) centered = -centered;
        sum += centered;
        if (centered > maxVal) maxVal = centered;
    }

    _audioLevel = maxVal;

    // Smooth the audio level
    uint8_t rawByte = map(constrain(_audioLevel, AUDIO_NOISE_FLOOR, 2048),
                          AUDIO_NOISE_FLOOR, 2048, 0, 255);
    _audioSmooth = (_audioSmooth * 3 + rawByte) / 4; // exponential smoothing

    // Peak tracking
    if (_audioSmooth > _peak) {
        _peak = _audioSmooth;
        _peakDecay = 20;
    } else {
        if (_peakDecay > 0) {
            _peakDecay--;
        } else {
            if (_peak > 2) _peak -= 2;
            else _peak = 0;
        }
    }
}

void LEDEffects::setAudioLevel(uint16_t level) {
    _audioLevel = level;
    uint8_t rawByte = map(constrain(level, AUDIO_NOISE_FLOOR, 2048),
                          AUDIO_NOISE_FLOOR, 2048, 0, 255);
    _audioSmooth = (_audioSmooth * 3 + rawByte) / 4;
}

uint8_t LEDEffects::getAudioByte() {
    return _audioSmooth;
}

// ============================================
// Effect: OFF
// ============================================
void LEDEffects::effectOff() {
    FastLED.clear();
}

// ============================================
// Effect: VU Meter (Green -> Yellow -> Red)
// ============================================
void LEDEffects::effectVUMeter() {
    int litLeds = map(_audioSmooth, 0, 255, 0, LED_COUNT);
    int peakLed = map(_peak, 0, 255, 0, LED_COUNT - 1);

    for (int i = 0; i < LED_COUNT; i++) {
        if (i < litLeds) {
            // Gradient: green -> yellow -> red
            if (i < LED_COUNT * 6 / 10) {
                _leds[i] = CRGB::Green;
            } else if (i < LED_COUNT * 8 / 10) {
                _leds[i] = CRGB::Yellow;
            } else {
                _leds[i] = CRGB::Red;
            }
        } else {
            _leds[i] = CRGB::Black;
        }
    }

    // Peak dot
    if (peakLed > 0 && peakLed < LED_COUNT) {
        _leds[peakLed] = CRGB::White;
    }
}

// ============================================
// Effect: Color Spectrum
// ============================================
void LEDEffects::effectSpectrum() {
    int litLeds = map(_audioSmooth, 0, 255, 1, LED_COUNT);

    for (int i = 0; i < LED_COUNT; i++) {
        if (i < litLeds) {
            // Each LED gets a different hue, shifted by audio
            uint8_t hue = _hue + (i * 255 / LED_COUNT);
            uint8_t brightness = map(i, 0, litLeds, 255, 100);
            _leds[i] = CHSV(hue, 255, brightness);
        } else {
            _leds[i].fadeToBlackBy(40);
        }
    }
}

// ============================================
// Effect: Pulse (beat detection)
// ============================================
void LEDEffects::effectPulse() {
    static uint8_t lastSmooth = 0;
    static uint8_t pulseVal = 0;
    static uint8_t pulseHue = 0;

    // Simple beat detection: sharp rise in audio
    if (_audioSmooth > lastSmooth + 30) {
        pulseVal = 255;
        pulseHue += 32;
    }
    lastSmooth = _audioSmooth;

    // Decay
    if (pulseVal > 4) pulseVal -= 4;
    else pulseVal = 0;

    // Base ambient glow from audio level
    uint8_t ambient = _audioSmooth / 6;

    for (int i = 0; i < LED_COUNT; i++) {
        uint8_t val = qadd8(pulseVal, ambient);
        _leds[i] = CHSV(pulseHue, 255, val);
    }
}

// ============================================
// Effect: Rainbow with audio intensity
// ============================================
void LEDEffects::effectRainbow() {
    uint8_t intensity = map(_audioSmooth, 0, 255, 40, 255);
    uint8_t speed = map(_audioSmooth, 0, 255, 1, 8);

    fill_rainbow(_leds, LED_COUNT, _hue * speed, 255 / LED_COUNT);

    // Modulate brightness by audio
    for (int i = 0; i < LED_COUNT; i++) {
        _leds[i].nscale8(intensity);
    }
}

// ============================================
// Effect: Fire
// ============================================
void LEDEffects::effectFire() {
    static uint8_t heat[LED_COUNT];

    // Audio modulates cooling and sparking
    uint8_t cooling = map(_audioSmooth, 0, 255, 20, 80);
    uint8_t sparking = map(_audioSmooth, 0, 255, 50, 240);

    // Cool down
    for (int i = 0; i < LED_COUNT; i++) {
        heat[i] = qsub8(heat[i], random8(0, ((cooling * 10) / LED_COUNT) + 2));
    }

    // Heat drift up
    for (int k = LED_COUNT - 1; k >= 2; k--) {
        heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
    }

    // Random sparks near bottom
    if (random8() < sparking) {
        int y = random8(7);
        heat[y] = qadd8(heat[y], random8(160, 255));
    }

    // Map heat to colors
    for (int j = 0; j < LED_COUNT; j++) {
        _leds[j] = heatColor(heat[j]);
    }
}

CRGB LEDEffects::heatColor(uint8_t temperature) {
    uint8_t t192 = scale8_video(temperature, 191);
    uint8_t heatramp = t192 & 0x3F;
    heatramp <<= 2;

    if (t192 & 0x80) {
        return CRGB(255, 255, heatramp);
    } else if (t192 & 0x40) {
        return CRGB(255, heatramp, 0);
    } else {
        return CRGB(heatramp, 0, 0);
    }
}

// ============================================
// Control
// ============================================
void LEDEffects::setBrightness(uint8_t brightness) {
    _brightness = brightness;
    FastLED.setBrightness(brightness);
}

uint8_t LEDEffects::getBrightness() const {
    return _brightness;
}

void LEDEffects::setEffect(LEDEffect effect) {
    _currentEffect = effect;
    FastLED.clear();
    FastLED.show();
    Serial.printf("[LED] Efeito: %s\n", getEffectName());
}

LEDEffect LEDEffects::getEffect() const {
    return _currentEffect;
}

void LEDEffects::nextEffect() {
    int next = ((int)_currentEffect + 1) % LED_EFFECT_COUNT;
    setEffect((LEDEffect)next);
}

void LEDEffects::cycleBrightness() {
    // Cycle: 50 -> 100 -> 150 -> 200 -> 250 -> 50
    _brightness += 50;
    if (_brightness > 250) _brightness = 50;
    FastLED.setBrightness(_brightness);
    Serial.printf("[LED] Brilho: %d\n", _brightness);
}

const char* LEDEffects::getEffectName() const {
    switch (_currentEffect) {
    case LED_OFF:       return "OFF";
    case LED_VU_METER:  return "VU METER";
    case LED_SPECTRUM:  return "SPECTRUM";
    case LED_PULSE:     return "PULSE";
    case LED_RAINBOW:   return "RAINBOW";
    case LED_FIRE:      return "FIRE";
    default:            return "---";
    }
}
