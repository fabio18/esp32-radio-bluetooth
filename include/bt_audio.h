/**
 * ESP32 Radio Bluetooth - Bluetooth A2DP Audio Sink
 * Receives audio from Alexa (or any BT source) and outputs via I2S DAC
 */

#ifndef BT_AUDIO_H
#define BT_AUDIO_H

#include <Arduino.h>
#include "BluetoothA2DPSink.h"
#include "config.h"

// Callback types
typedef void (*BTConnectionCallback)(bool connected);
typedef void (*BTPlaybackCallback)(bool playing);

class BTAudio {
public:
    BTAudio();

    void begin(const char* deviceName = BT_DEVICE_NAME);
    void end();
    bool isConnected() const;
    bool isPlaying() const;
    const char* getConnectedDeviceName() const;

    void setVolume(uint8_t volume);
    uint8_t getVolume() const;

    void setConnectionCallback(BTConnectionCallback cb);
    void setPlaybackCallback(BTPlaybackCallback cb);

    // AVRC controls (if source supports)
    void play();
    void pause();
    void next();
    void previous();

private:
    BluetoothA2DPSink _a2dpSink;
    bool _connected;
    bool _playing;
    uint8_t _volume;
    char _connectedDevice[64];

    BTConnectionCallback _connectionCb;
    BTPlaybackCallback _playbackCb;

    static BTAudio* _instance;
    static void connectionStateCallback(esp_a2d_connection_state_t state, void* obj);
    static void audioStateCallback(esp_a2d_audio_state_t state, void* obj);
};

#endif
