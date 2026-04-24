/**
 * TEA5767 FM Radio Module Driver
 * I2C communication for ESP32
 */

#include "TEA5767.h"

TEA5767::TEA5767()
    : _frequency(TEA5767_DEFAULT_FREQ),
      _muted(false),
      _standby(false),
      _stereoMode(true),
      _searchStopLevel(TEA5767_SEARCH_MID),
      _presetCount(0),
      _currentPreset(-1)
{
    memset(_writeData, 0, sizeof(_writeData));
    memset(_readData, 0, sizeof(_readData));

    // Default presets (popular Brazilian FM stations)
    float defaultPresets[] = {
        89.1, 91.3, 93.7, 96.1, 98.3, 100.9, 101.1, 103.3, 105.1, 107.5
    };
    _presetCount = sizeof(defaultPresets) / sizeof(defaultPresets[0]);
    for (int i = 0; i < _presetCount; i++) {
        _presets[i] = defaultPresets[i];
    }
}

bool TEA5767::begin(int sda, int scl) {
    Wire.begin(sda, scl);
    Wire.beginTransmission(TEA5767_ADDR);
    uint8_t error = Wire.endTransmission();
    if (error != 0) {
        return false;
    }
    setFrequency(_frequency);
    return true;
}

uint16_t TEA5767::calculatePLL(float freq) const {
    // PLL formula: PLL = 4 * (freq_RF + freq_IF) / freq_ref
    // freq_IF = 225 kHz, freq_ref = 32.768 kHz
    return (uint16_t)(4.0 * (freq * 1000000.0 + 225000.0) / 32768.0);
}

float TEA5767::calculateFrequency(uint16_t pll) const {
    return ((pll * 32768.0) / 4.0 - 225000.0) / 1000000.0;
}

void TEA5767::transmit() {
    uint16_t pll = calculatePLL(_frequency);

    _writeData[0] = (pll >> 8) & 0x3F;
    if (_muted) _writeData[0] |= 0x80;

    _writeData[1] = pll & 0xFF;

    _writeData[2] = 0xB0; // SUD=1, SSL=10 (mid), HLSI=1
    _writeData[2] &= 0x1F;
    _writeData[2] |= (_searchStopLevel & 0x03) << 5;
    _writeData[2] |= 0x10; // HLSI = 1 (high side injection)

    _writeData[3] = 0x10; // XTAL = 32.768 kHz
    if (!_stereoMode) _writeData[3] |= 0x08; // Force mono

    if (_standby) _writeData[3] |= 0x40; // Standby

    _writeData[4] = 0x00;

    Wire.beginTransmission(TEA5767_ADDR);
    Wire.write(_writeData, 5);
    Wire.endTransmission();
}

void TEA5767::readStatus() {
    Wire.requestFrom((uint8_t)TEA5767_ADDR, (uint8_t)5);
    for (int i = 0; i < 5 && Wire.available(); i++) {
        _readData[i] = Wire.read();
    }
}

void TEA5767::setFrequency(float freq) {
    if (freq < TEA5767_FM_BAND_LOW) freq = TEA5767_FM_BAND_LOW;
    if (freq > TEA5767_FM_BAND_HIGH) freq = TEA5767_FM_BAND_HIGH;
    _frequency = freq;
    transmit();
}

float TEA5767::getFrequency() const {
    return _frequency;
}

void TEA5767::seekUp() {
    uint16_t pll = calculatePLL(_frequency);
    _writeData[0] = ((pll >> 8) & 0x3F) | 0x40; // SM = 1 (search mode)
    if (_muted) _writeData[0] |= 0x80;
    _writeData[1] = pll & 0xFF;
    _writeData[2] = 0xB0; // SUD=1 (up), SSL=10
    _writeData[2] |= 0x80; // Search up
    _writeData[3] = 0x10;
    _writeData[4] = 0x00;

    Wire.beginTransmission(TEA5767_ADDR);
    Wire.write(_writeData, 5);
    Wire.endTransmission();

    delay(500);
    readStatus();

    uint16_t newPll = ((_readData[0] & 0x3F) << 8) | _readData[1];
    _frequency = calculateFrequency(newPll);
}

void TEA5767::seekDown() {
    uint16_t pll = calculatePLL(_frequency);
    _writeData[0] = ((pll >> 8) & 0x3F) | 0x40; // SM = 1
    if (_muted) _writeData[0] |= 0x80;
    _writeData[1] = pll & 0xFF;
    _writeData[2] = 0x30; // SUD=0 (down), SSL=10
    _writeData[3] = 0x10;
    _writeData[4] = 0x00;

    Wire.beginTransmission(TEA5767_ADDR);
    Wire.write(_writeData, 5);
    Wire.endTransmission();

    delay(500);
    readStatus();

    uint16_t newPll = ((_readData[0] & 0x3F) << 8) | _readData[1];
    _frequency = calculateFrequency(newPll);
}

void TEA5767::setMute(bool mute) {
    _muted = mute;
    transmit();
}

bool TEA5767::isMuted() const {
    return _muted;
}

void TEA5767::setStandby(bool standby) {
    _standby = standby;
    transmit();
}

bool TEA5767::isStandby() const {
    return _standby;
}

void TEA5767::setStereo(bool stereo) {
    _stereoMode = stereo;
    transmit();
}

void TEA5767::setSearchStopLevel(uint8_t level) {
    _searchStopLevel = level;
    transmit();
}

TEA5767_Status TEA5767::getStatus() {
    readStatus();

    TEA5767_Status status;
    uint16_t pll = ((_readData[0] & 0x3F) << 8) | _readData[1];
    status.frequency = calculateFrequency(pll);
    status.ready = (_readData[0] & 0x80) != 0;
    status.bandLimitReached = (_readData[0] & 0x40) != 0;
    status.stereo = (_readData[2] & 0x80) != 0;
    status.signalLevel = (_readData[3] >> 4) & 0x0F;

    return status;
}

void TEA5767::nextPreset() {
    if (_presetCount == 0) return;
    _currentPreset = (_currentPreset + 1) % _presetCount;
    setFrequency(_presets[_currentPreset]);
}

void TEA5767::prevPreset() {
    if (_presetCount == 0) return;
    _currentPreset--;
    if (_currentPreset < 0) _currentPreset = _presetCount - 1;
    setFrequency(_presets[_currentPreset]);
}

void TEA5767::addPreset(float freq) {
    if (_presetCount >= MAX_PRESETS) return;
    _presets[_presetCount++] = freq;
}

void TEA5767::removePreset(int index) {
    if (index < 0 || index >= _presetCount) return;
    for (int i = index; i < _presetCount - 1; i++) {
        _presets[i] = _presets[i + 1];
    }
    _presetCount--;
    if (_currentPreset >= _presetCount) _currentPreset = _presetCount - 1;
}

float TEA5767::getPreset(int index) const {
    if (index < 0 || index >= _presetCount) return 0;
    return _presets[index];
}

int TEA5767::getPresetCount() const {
    return _presetCount;
}

int TEA5767::getCurrentPresetIndex() const {
    return _currentPreset;
}
