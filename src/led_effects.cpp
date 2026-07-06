/**
 * ESP32 Kindlelaser Max 260W - LED Status Effects
 * WS2812B LED strip para indicação visual do status da máquina
 *
 * Status → Efeito:
 *   IDLE     → Azul pulsante lento (respiração)
 *   READY    → Verde fixo
 *   CUTTING  → Vermelho pulsante rápido
 *   WARNING  → Amarelo piscante
 *   ERROR    → Vermelho piscante rápido
 *   COOLDOWN → Azul gradiente varrendo
 */

#include "led_effects.h"

LEDEffects::LEDEffects()
    : _currentStatus(LED_STATUS_OFF),
      _brightness(LED_BRIGHTNESS),
      _hue(0),
      _lastUpdate(0),
      _pulseVal(0),
      _pulseDir(true)
{
}

void LEDEffects::begin() {
    FastLED.addLeds<LED_TYPE, LED_PIN, LED_COLOR_ORDER>(_leds, LED_COUNT);
    FastLED.setBrightness(_brightness);
    FastLED.clear();
    FastLED.show();

    Serial.printf("[LED] Status strip: %d LEDs on GPIO %d\n", LED_COUNT, LED_PIN);
}

void LEDEffects::update() {
    unsigned long now = millis();
    if (now - _lastUpdate < LED_UPDATE_MS) return;
    _lastUpdate = now;

    _hue++;

    switch (_currentStatus) {
    case LED_STATUS_OFF:      effectOff(); break;
    case LED_STATUS_IDLE:     effectIdle(); break;
    case LED_STATUS_READY:    effectReady(); break;
    case LED_STATUS_CUTTING:  effectCutting(); break;
    case LED_STATUS_WARNING:  effectWarning(); break;
    case LED_STATUS_ERROR:    effectError(); break;
    case LED_STATUS_COOLDOWN: effectCooldown(); break;
    default:                  effectOff(); break;
    }

    FastLED.show();
}

// ============================================
// Status Control
// ============================================
void LEDEffects::setStatus(LEDStatus status) {
    if (_currentStatus != status) {
        _currentStatus = status;
        _pulseVal = 0;
        _pulseDir = true;
    }
}

LEDStatus LEDEffects::getStatus() const {
    return _currentStatus;
}

const char* LEDEffects::getStatusName() const {
    switch (_currentStatus) {
    case LED_STATUS_OFF:      return "OFF";
    case LED_STATUS_IDLE:     return "IDLE";
    case LED_STATUS_READY:    return "PRONTO";
    case LED_STATUS_CUTTING:  return "CORTANDO";
    case LED_STATUS_WARNING:  return "AVISO";
    case LED_STATUS_ERROR:    return "ERRO";
    case LED_STATUS_COOLDOWN: return "RESFRIANDO";
    default:                  return "---";
    }
}

// ============================================
// Brightness
// ============================================
void LEDEffects::setBrightness(uint8_t brightness) {
    _brightness = brightness;
    FastLED.setBrightness(brightness);
}

uint8_t LEDEffects::getBrightness() const {
    return _brightness;
}

// ============================================
// Effect: OFF
// ============================================
void LEDEffects::effectOff() {
    FastLED.clear();
}

// ============================================
// Effect: IDLE - Blue breathing (slow pulse)
// ============================================
void LEDEffects::effectIdle() {
    // Slow breathing effect
    if (_pulseDir) {
        _pulseVal += 2;
        if (_pulseVal >= 200) _pulseDir = false;
    } else {
        _pulseVal -= 2;
        if (_pulseVal <= 20) _pulseDir = true;
    }

    for (int i = 0; i < LED_COUNT; i++) {
        _leds[i] = CHSV(160, 255, _pulseVal); // Blue hue
    }
}

// ============================================
// Effect: READY - Solid green
// ============================================
void LEDEffects::effectReady() {
    for (int i = 0; i < LED_COUNT; i++) {
        _leds[i] = CRGB::Green;
    }
}

// ============================================
// Effect: CUTTING - Red fast pulse
// ============================================
void LEDEffects::effectCutting() {
    // Fast pulse during cutting
    if (_pulseDir) {
        _pulseVal += 15;
        if (_pulseVal >= 255) {
            _pulseVal = 255;
            _pulseDir = false;
        }
    } else {
        _pulseVal -= 15;
        if (_pulseVal <= 50) {
            _pulseVal = 50;
            _pulseDir = true;
        }
    }

    for (int i = 0; i < LED_COUNT; i++) {
        _leds[i] = CRGB(_pulseVal, 0, 0); // Red
    }
}

// ============================================
// Effect: WARNING - Yellow blink
// ============================================
void LEDEffects::effectWarning() {
    // Blink on/off every ~500ms (10 updates * 50ms)
    bool on = (_hue / 10) % 2 == 0;

    for (int i = 0; i < LED_COUNT; i++) {
        if (on) {
            _leds[i] = CRGB::Yellow;
        } else {
            _leds[i] = CRGB::Black;
        }
    }
}

// ============================================
// Effect: ERROR - Red fast blink
// ============================================
void LEDEffects::effectError() {
    // Fast blink every ~250ms (5 updates * 50ms)
    bool on = (_hue / 5) % 2 == 0;

    for (int i = 0; i < LED_COUNT; i++) {
        if (on) {
            _leds[i] = CRGB::Red;
        } else {
            _leds[i] = CRGB::Black;
        }
    }
}

// ============================================
// Effect: COOLDOWN - Blue gradient sweep
// ============================================
void LEDEffects::effectCooldown() {
    for (int i = 0; i < LED_COUNT; i++) {
        uint8_t pos = (i * 255 / LED_COUNT + _hue * 2) & 0xFF;
        _leds[i] = CHSV(160, 255, sin8(pos)); // Blue with sine wave
    }
}
