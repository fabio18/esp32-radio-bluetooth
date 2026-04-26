/**
 * ESP32 Radio Bluetooth - Configuration
 * Visual estilo Pioneer automotivo
 * Controle 100% por tela touch (sem botões físicos)
 * LED WS2812B reativo ao som
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
// Touch Screen Pins (XPT2046 - shared SPI)
// ============================================
#define TOUCH_CS        27
#define TOUCH_IRQ       34

// ============================================
// Audio Output (I2S DAC)
// ============================================
#define I2S_BCLK        5
#define I2S_LRC         17
#define I2S_DOUT        16

// ============================================
// WS2812B LED Strip
// ============================================
#define LED_PIN         13
#define LED_COUNT       30
#define LED_BRIGHTNESS  150
#define LED_TYPE        WS2812B
#define LED_COLOR_ORDER GRB

// Audio input for LED reactivity (ADC)
#define AUDIO_IN_PIN    36    // VP (ADC1_CH0) - connect to audio signal

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
// Touch Calibration
// ============================================
#define TOUCH_MIN_X     300
#define TOUCH_MAX_X     3800
#define TOUCH_MIN_Y     300
#define TOUCH_MAX_Y     3800

// ============================================
// Pioneer Color Scheme (RGB565)
// ============================================

// Backgrounds
#define COLOR_BG            0x0001  // Near-black (very dark blue)
#define COLOR_HEADER_BG     0x0011  // Dark navy
#define COLOR_BTN_BG        0x0841  // Dark charcoal-blue
#define COLOR_BTN_PRESSED   0x1082  // Lighter pressed
#define COLOR_PANEL_BG      0x0821  // Subtle dark panel

// Primary neon colors (Pioneer cyan/blue glow)
#define COLOR_NEON_CYAN     0x07FF  // Bright cyan (main frequency)
#define COLOR_NEON_BLUE     0x03BF  // Electric blue
#define COLOR_NEON_GREEN    0x07E0  // Neon green
#define COLOR_NEON_ORANGE   0xFCA0  // Warm orange
#define COLOR_NEON_RED      0xF800  // Red (warnings/mute)
#define COLOR_NEON_MAGENTA  0xF81F  // Magenta accent
#define COLOR_NEON_WHITE    0xFFFF  // Bright white

// UI element colors
#define COLOR_FREQ          0x07FF  // Cyan - frequency digits
#define COLOR_FREQ_GLOW     0x033F  // Darker cyan - glow around freq
#define COLOR_TEXT          0xBDF7  // Light silver text
#define COLOR_TEXT_DIM      0x52AA  // Dim gray text
#define COLOR_TEXT_BRIGHT   0xFFFF  // Bright white
#define COLOR_DIVIDER       0x0842  // Very subtle dark divider

// Signal & status
#define COLOR_SIGNAL_HIGH   0x07E0  // Green - strong signal
#define COLOR_SIGNAL_MID    0x07E0  // Green-yellow
#define COLOR_SIGNAL_LOW    0xFCA0  // Orange - weak signal
#define COLOR_SIGNAL_NONE   0xF800  // Red - no signal
#define COLOR_STEREO        0x07FF  // Cyan

// Buttons
#define COLOR_BTN_BORDER    0x02DF  // Blue border glow
#define COLOR_BTN_TEXT      0x07FF  // Cyan text on buttons
#define COLOR_BTN_ACTIVE    0x03BF  // Active/selected blue
#define COLOR_BTN_MODE_FM   0x07E0  // Green for FM mode button
#define COLOR_BTN_MODE_BT   0x03BF  // Blue for BT mode button

// Volume
#define COLOR_VOL_BAR       0x07E0  // Green volume fill
#define COLOR_VOL_HIGH      0xFCA0  // Orange when volume high
#define COLOR_VOL_MAX       0xF800  // Red when maxed
#define COLOR_VOL_BG        0x1082  // Dark volume track

// EQ Visualizer
#define COLOR_EQ_LOW        0x07E0  // Green (low bars)
#define COLOR_EQ_MID        0x07FF  // Cyan (mid bars)
#define COLOR_EQ_HIGH       0x03BF  // Blue (high bars)
#define COLOR_EQ_PEAK       0xFFE0  // Yellow peak
#define COLOR_EQ_BG         0x0841  // Dark EQ background

// BT specific
#define COLOR_BT_ICON       0x03BF  // Blue BT icon
#define COLOR_BT_CONNECTED  0x07E0  // Green when connected
#define COLOR_BT_WAITING    0xFCA0  // Orange when waiting

// ============================================
// System Constants
// ============================================
#define TOUCH_DEBOUNCE_MS   300
#define DISPLAY_UPDATE_MS   100
#define STATUS_READ_MS      500
#define EQ_UPDATE_MS        80
#define FREQ_STEP           0.1
#define VOLUME_DEFAULT      80
#define VOLUME_MAX          100
#define VOLUME_STEP         5
#define EQ_BARS             16
#define EQ_MAX_HEIGHT       30
#define LED_UPDATE_MS       20
#define AUDIO_SAMPLES       64
#define AUDIO_NOISE_FLOOR   50

// ============================================
// Radio Modes
// ============================================
enum RadioMode {
    MODE_FM = 0,
    MODE_BLUETOOTH = 1
};

// ============================================
// Touch Button IDs
// ============================================
enum TouchButton {
    BTN_NONE = 0,
    BTN_SEEK_DOWN,
    BTN_SEEK_UP,
    BTN_PRESET_PREV,
    BTN_PRESET_NEXT,
    BTN_VOL_DOWN,
    BTN_VOL_UP,
    BTN_MUTE,
    BTN_MODE,
    BTN_PLAY_PAUSE,
    BTN_LED_MODE,
    BTN_LED_BRIGHT
};

// ============================================
// LED Effect Modes
// ============================================
enum LEDEffect {
    LED_OFF = 0,
    LED_VU_METER,       // Green->Yellow->Red VU meter
    LED_SPECTRUM,       // Color spectrum reacting to audio
    LED_PULSE,          // Pulsing color with beat
    LED_RAINBOW,        // Rainbow cycle with audio intensity
    LED_FIRE,           // Fire effect with audio
    LED_EFFECT_COUNT
};

#endif
