/**
 * ESP32 Kindlelaser Max 260W - LED Status Effects
 * WS2812B LED strip for machine status indication
 */

#ifndef LED_EFFECTS_H
#define LED_EFFECTS_H

#include <Arduino.h>
#include <FastLED.h>
#include "config.h"

class LEDEffects {
public:
    LEDEffects();

    void begin();
    void update();

    // Status control
    void setStatus(LEDStatus status);
    LEDStatus getStatus() const;
    const char* getStatusName() const;

    // Brightness
    void setBrightness(uint8_t brightness);
    uint8_t getBrightness() const;

private:
    CRGB _leds[LED_COUNT];
    LEDStatus _currentStatus;
    uint8_t _brightness;
    uint8_t _hue;
    unsigned long _lastUpdate;
    uint8_t _pulseVal;
    bool _pulseDir;

    // Effects
    void effectOff();
    void effectIdle();       // Blue breathing
    void effectReady();      // Solid green
    void effectCutting();    // Red fast pulse
    void effectWarning();    // Yellow blink
    void effectError();      // Red blink
    void effectCooldown();   // Blue gradient sweep
};

#endif
