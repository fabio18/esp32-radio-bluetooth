/**
 * ESP32 Radio Bluetooth - WS2812B LED Effects
 * LED strip reativa ao som com múltiplos efeitos
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
    void setBrightness(uint8_t brightness);
    uint8_t getBrightness() const;
    void setEffect(LEDEffect effect);
    LEDEffect getEffect() const;
    void nextEffect();
    void cycleBrightness();
    void setAudioLevel(uint16_t level);

    const char* getEffectName() const;

private:
    CRGB _leds[LED_COUNT];
    LEDEffect _currentEffect;
    uint8_t _brightness;
    uint16_t _audioLevel;     // 0-4095 raw ADC
    uint8_t _audioSmooth;     // 0-255 smoothed
    uint8_t _peak;
    uint8_t _peakDecay;
    uint8_t _hue;
    unsigned long _lastUpdate;

    // Audio processing
    void readAudio();
    uint8_t getAudioByte();

    // Effect functions
    void effectOff();
    void effectVUMeter();
    void effectSpectrum();
    void effectPulse();
    void effectRainbow();
    void effectFire();

    // Helpers
    CRGB heatColor(uint8_t temperature);
};

#endif
