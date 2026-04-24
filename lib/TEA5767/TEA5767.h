/**
 * TEA5767 FM Radio Module Driver
 * I2C communication for ESP32
 */

#ifndef TEA5767_H
#define TEA5767_H

#include <Arduino.h>
#include <Wire.h>

#define TEA5767_ADDR 0x60

// Band limits
#define TEA5767_FM_BAND_LOW   76.0
#define TEA5767_FM_BAND_HIGH  108.0
#define TEA5767_DEFAULT_FREQ  101.1

// Search direction
#define TEA5767_SEARCH_UP     1
#define TEA5767_SEARCH_DOWN   0

// Search stop level (ADC)
#define TEA5767_SEARCH_LOW    1
#define TEA5767_SEARCH_MID    2
#define TEA5767_SEARCH_HIGH   3

struct TEA5767_Status {
    float frequency;
    bool stereo;
    uint8_t signalLevel;
    bool bandLimitReached;
    bool ready;
};

class TEA5767 {
public:
    TEA5767();

    bool begin(int sda = 21, int scl = 22);
    void setFrequency(float freq);
    float getFrequency() const;
    void seekUp();
    void seekDown();
    void setMute(bool mute);
    bool isMuted() const;
    void setStandby(bool standby);
    bool isStandby() const;
    void setStereo(bool stereo);
    void setSearchStopLevel(uint8_t level);
    TEA5767_Status getStatus();

    // Preset management
    void nextPreset();
    void prevPreset();
    void addPreset(float freq);
    void removePreset(int index);
    float getPreset(int index) const;
    int getPresetCount() const;
    int getCurrentPresetIndex() const;

private:
    void transmit();
    void readStatus();
    float calculateFrequency(uint16_t pll) const;
    uint16_t calculatePLL(float freq) const;

    uint8_t _writeData[5];
    uint8_t _readData[5];
    float _frequency;
    bool _muted;
    bool _standby;
    bool _stereoMode;
    uint8_t _searchStopLevel;

    static const int MAX_PRESETS = 30;
    float _presets[MAX_PRESETS];
    int _presetCount;
    int _currentPreset;
};

#endif
