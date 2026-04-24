/**
 * ESP32 Radio Bluetooth - Bluetooth A2DP Audio Sink
 * Receives audio from Alexa (or any BT source) and outputs via I2S DAC
 */

#include "bt_audio.h"

BTAudio* BTAudio::_instance = nullptr;

BTAudio::BTAudio()
    : _connected(false),
      _playing(false),
      _volume(VOLUME_DEFAULT),
      _connectionCb(nullptr),
      _playbackCb(nullptr)
{
    memset(_connectedDevice, 0, sizeof(_connectedDevice));
    _instance = this;
}

void BTAudio::begin(const char* deviceName) {
    // Configure I2S pins
    i2s_pin_config_t pinConfig = {
        .bck_io_num = I2S_BCLK,
        .ws_io_num = I2S_LRC,
        .data_out_num = I2S_DOUT,
        .data_in_num = I2S_PIN_NO_CHANGE
    };
    _a2dpSink.set_pin_config(pinConfig);

    // Set callbacks
    _a2dpSink.set_on_connection_state_changed(connectionStateCallback);
    _a2dpSink.set_on_audio_state_changed(audioStateCallback);

    // Set volume
    _a2dpSink.set_volume(_volume * 127 / 100);

    // Start Bluetooth A2DP Sink
    _a2dpSink.start(deviceName);

    Serial.printf("[BT] A2DP Sink started: %s\n", deviceName);
}

void BTAudio::end() {
    _a2dpSink.end(true);
    _connected = false;
    _playing = false;
    Serial.println("[BT] A2DP Sink stopped");
}

bool BTAudio::isConnected() const {
    return _connected;
}

bool BTAudio::isPlaying() const {
    return _playing;
}

const char* BTAudio::getConnectedDeviceName() const {
    return _connectedDevice;
}

void BTAudio::setVolume(uint8_t volume) {
    if (volume > VOLUME_MAX) volume = VOLUME_MAX;
    _volume = volume;
    _a2dpSink.set_volume(volume * 127 / 100);
}

uint8_t BTAudio::getVolume() const {
    return _volume;
}

void BTAudio::setConnectionCallback(BTConnectionCallback cb) {
    _connectionCb = cb;
}

void BTAudio::setPlaybackCallback(BTPlaybackCallback cb) {
    _playbackCb = cb;
}

void BTAudio::play() {
    _a2dpSink.play();
}

void BTAudio::pause() {
    _a2dpSink.pause();
}

void BTAudio::next() {
    _a2dpSink.next();
}

void BTAudio::previous() {
    _a2dpSink.previous();
}

void BTAudio::connectionStateCallback(esp_a2d_connection_state_t state, void* obj) {
    if (!_instance) return;

    if (state == ESP_A2D_CONNECTION_STATE_CONNECTED) {
        _instance->_connected = true;
        strncpy(_instance->_connectedDevice, "Alexa", sizeof(_instance->_connectedDevice) - 1);
        Serial.println("[BT] Device connected");
        if (_instance->_connectionCb) _instance->_connectionCb(true);
    } else if (state == ESP_A2D_CONNECTION_STATE_DISCONNECTED) {
        _instance->_connected = false;
        _instance->_playing = false;
        memset(_instance->_connectedDevice, 0, sizeof(_instance->_connectedDevice));
        Serial.println("[BT] Device disconnected");
        if (_instance->_connectionCb) _instance->_connectionCb(false);
    }
}

void BTAudio::audioStateCallback(esp_a2d_audio_state_t state, void* obj) {
    if (!_instance) return;

    if (state == ESP_A2D_AUDIO_STATE_STARTED) {
        _instance->_playing = true;
        Serial.println("[BT] Audio started");
        if (_instance->_playbackCb) _instance->_playbackCb(true);
    } else if (state == ESP_A2D_AUDIO_STATE_REMOTE_SUSPEND ||
               state == ESP_A2D_AUDIO_STATE_STOPPED) {
        _instance->_playing = false;
        Serial.println("[BT] Audio stopped");
        if (_instance->_playbackCb) _instance->_playbackCb(false);
    }
}
