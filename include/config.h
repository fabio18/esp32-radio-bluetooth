/**
 * ESP32 Radio Bluetooth - Configuration
 * Pin definitions and system constants
 */

#ifndef CONFIG_H
#define CONFIG_H

// ============================================
// I2C Pins (TEA5767)
// ============================================
#define I2C_SDA         21
#define I2C_SCL         22

// ============================================
// SPI Pins (ILI9341 TFT) - defined in platformio.ini
// ============================================
// TFT_MOSI  23
// TFT_MISO  19
// TFT_SCLK  18
// TFT_CS    15
// TFT_DC     2
// TFT_RST    4

// ============================================
// Touch / Button Pins
// ============================================
#define BTN_MODE        35    // Mode: FM / Bluetooth
#define BTN_SEEK_UP     34    // Seek up / Volume up
#define BTN_SEEK_DOWN   32    // Seek down / Volume down
#define BTN_MUTE        33    // Mute toggle
#define BTN_PRESET_NEXT 25    // Next preset
#define BTN_PRESET_PREV 26    // Previous preset

// ============================================
// Rotary Encoder (optional - for tuning)
// ============================================
#define ENCODER_CLK     27
#define ENCODER_DT      14
#define ENCODER_SW      12

// ============================================
// Audio Output (I2S DAC)
// ============================================
#define I2S_BCLK        5
#define I2S_LRC         17
#define I2S_DOUT        16

// ============================================
// Bluetooth Configuration
// ============================================
#define BT_DEVICE_NAME  "ESP32 Radio"

// ============================================
// Display Configuration
// ============================================
#define SCREEN_WIDTH    240
#define SCREEN_HEIGHT   320

// ============================================
// Colors (RGB565)
// ============================================
#define COLOR_BG            0x0000  // Black
#define COLOR_PRIMARY       0x07FF  // Cyan
#define COLOR_SECONDARY     0xFFE0  // Yellow
#define COLOR_ACCENT        0xF81F  // Magenta
#define COLOR_TEXT          0xFFFF  // White
#define COLOR_TEXT_DIM      0x7BEF  // Gray
#define COLOR_FREQ          0x07FF  // Cyan
#define COLOR_SIGNAL        0x07E0  // Green
#define COLOR_SIGNAL_LOW    0xFBE0  // Orange
#define COLOR_SIGNAL_NONE   0xF800  // Red
#define COLOR_BT_ACTIVE     0x001F  // Blue
#define COLOR_FM_ACTIVE     0x07E0  // Green
#define COLOR_MUTED         0xF800  // Red
#define COLOR_PRESET_BG     0x18E3  // Dark gray
#define COLOR_PRESET_SEL    0x04FF  // Light blue
#define COLOR_DIVIDER       0x3186  // Mid gray
#define COLOR_HEADER_BG     0x10A2  // Dark blue-gray
#define COLOR_VOLUME_BAR    0x07E0  // Green
#define COLOR_VOLUME_BG     0x2104  // Dark gray

// ============================================
// System Constants
// ============================================
#define DEBOUNCE_MS         200
#define DISPLAY_UPDATE_MS   250
#define STATUS_READ_MS      500
#define FREQ_STEP           0.1
#define VOLUME_DEFAULT      80
#define VOLUME_MAX          100
#define VOLUME_STEP         5

// ============================================
// Radio Modes
// ============================================
enum RadioMode {
    MODE_FM = 0,
    MODE_BLUETOOTH = 1
};

#endif
