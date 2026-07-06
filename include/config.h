/**
 * ESP32 Kindlelaser Max 260W - Metal Cutting Controller
 * Painel de controle touch para corte de metal com laser CO2
 * Interface visual estilo industrial com monitoramento de segurança
 *
 * Adaptado do projeto ESP32 Radio Bluetooth
 */

#ifndef CONFIG_H
#define CONFIG_H

// ============================================
// Laser Control Pins
// ============================================
#define LASER_PWM_PIN       25    // PWM output → PSU laser (0-5V via level shifter)
#define LASER_ENABLE_PIN    26    // Laser ON/OFF (active HIGH via optocoupler)
#define AIR_ASSIST_PIN      14    // Solenoide ar comprimido (active HIGH)
#define EXHAUST_FAN_PIN     12    // Exaustor/ventilação (active HIGH)

// ============================================
// Safety Input Pins (all INPUT_PULLUP)
// ============================================
#define WATER_FLOW_PIN      33    // Sensor de fluxo de água (LOW = fluxo OK)
#define LID_SWITCH_PIN      32    // Chave de tampa (LOW = fechada)
#define ESTOP_PIN           39    // Botão de emergência (LOW = acionado)
#define TEMP_SENSOR_PIN     35    // NTC temperatura água (ADC input)

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
// WS2812B LED Strip (Status Indication)
// ============================================
#define LED_PIN         13
#define LED_COUNT       30
#define LED_BRIGHTNESS  150
#define LED_TYPE        WS2812B
#define LED_COLOR_ORDER GRB

// ============================================
// Communication (optional UART to Ruida controller)
// ============================================
#define RUIDA_RX_PIN    16    // UART2 RX from Ruida RDC6445
#define RUIDA_TX_PIN    17    // UART2 TX to Ruida RDC6445
#define RUIDA_BAUD      115200

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
// Laser PWM Configuration
// ============================================
#define LASER_PWM_FREQ      20000   // 20kHz PWM (compatível com PSU laser CO2)
#define LASER_PWM_CHANNEL   0
#define LASER_PWM_RESOLUTION 12     // 12-bit (0-4095)
#define LASER_PWM_MAX       4095    // Valor máximo do duty cycle

// ============================================
// Kindlelaser Max 260W Specifications
// ============================================
#define LASER_MAX_POWER_W   260     // Potência máxima em Watts
#define LASER_MIN_POWER_PCT 5       // Potência mínima operacional (%)
#define LASER_MAX_POWER_PCT 99      // Potência máxima segura (% - nunca 100%)
#define LASER_TUBE_MAX_MA   28      // Corrente máxima do tubo (mA)

// ============================================
// Cutting Parameters
// ============================================
#define POWER_DEFAULT       50      // Potência padrão (%)
#define POWER_MAX           99      // Máximo permitido (%)
#define POWER_STEP          5       // Incremento por toque
#define SPEED_DEFAULT       15      // Velocidade padrão (mm/s)
#define SPEED_MIN           1       // Velocidade mínima
#define SPEED_MAX           80      // Velocidade máxima (mm/s)
#define SPEED_STEP          1       // Incremento por toque

// ============================================
// Safety Thresholds
// ============================================
#define WATER_TEMP_MAX      35.0f   // Temperatura máxima da água (°C)
#define WATER_TEMP_WARNING  30.0f   // Aviso de temperatura (°C)
#define WATER_FLOW_TIMEOUT  2000    // Timeout sem fluxo (ms) → desliga laser
#define SAFETY_CHECK_MS     200     // Intervalo de verificação de segurança
#define LASER_WARMUP_MS     3000    // Tempo de aquecimento antes de corte
#define MAX_CONTINUOUS_CUT  300000  // Máximo corte contínuo 5min (ms) → pausa

// ============================================
// NTC Temperature Sensor
// ============================================
#define NTC_NOMINAL_R       10000   // Resistência a 25°C (10kΩ)
#define NTC_NOMINAL_T       25      // Temperatura nominal
#define NTC_BETA            3950    // Coeficiente Beta do NTC
#define NTC_SERIES_R        10000   // Resistor em série (10kΩ)

// ============================================
// Industrial Color Scheme (RGB565)
// ============================================

// Backgrounds
#define COLOR_BG            0x0001  // Near-black
#define COLOR_HEADER_BG     0x0011  // Dark navy
#define COLOR_BTN_BG        0x0841  // Dark charcoal
#define COLOR_BTN_PRESSED   0x1082  // Lighter pressed
#define COLOR_PANEL_BG      0x0821  // Dark panel

// Status Colors
#define COLOR_SAFE          0x07E0  // Green - sistema OK
#define COLOR_WARNING       0xFD20  // Orange - aviso
#define COLOR_DANGER        0xF800  // Red - perigo/erro
#define COLOR_CUTTING       0x07FF  // Cyan - cortando
#define COLOR_IDLE          0x03BF  // Blue - em espera
#define COLOR_DISABLED      0x4208  // Dark gray - desabilitado

// Primary Colors
#define COLOR_NEON_CYAN     0x07FF  // Bright cyan
#define COLOR_NEON_BLUE     0x03BF  // Electric blue
#define COLOR_NEON_GREEN    0x07E0  // Neon green
#define COLOR_NEON_ORANGE   0xFCA0  // Warm orange
#define COLOR_NEON_RED      0xF800  // Red
#define COLOR_NEON_MAGENTA  0xF81F  // Magenta accent
#define COLOR_NEON_WHITE    0xFFFF  // Bright white
#define COLOR_NEON_YELLOW   0xFFE0  // Yellow

// UI element colors
#define COLOR_POWER         0xF800  // Red - potência laser
#define COLOR_POWER_LOW     0x07E0  // Green - potência baixa
#define COLOR_POWER_MID     0xFCA0  // Orange - potência média
#define COLOR_POWER_HIGH    0xF800  // Red - potência alta
#define COLOR_SPEED         0x07FF  // Cyan - velocidade
#define COLOR_TEXT          0xBDF7  // Light silver text
#define COLOR_TEXT_DIM      0x52AA  // Dim gray text
#define COLOR_TEXT_BRIGHT   0xFFFF  // Bright white
#define COLOR_DIVIDER       0x0842  // Dark divider

// Buttons
#define COLOR_BTN_BORDER    0x02DF  // Blue border
#define COLOR_BTN_TEXT      0x07FF  // Cyan text
#define COLOR_BTN_ACTIVE    0x03BF  // Active blue
#define COLOR_BTN_LASER_ON  0xF800  // Red - laser ativo
#define COLOR_BTN_LASER_OFF 0x07E0  // Green - laser desligado
#define COLOR_BTN_AIR       0x07FF  // Cyan - ar comprimido

// Progress bars
#define COLOR_BAR_BG        0x1082  // Dark bar background
#define COLOR_BAR_POWER     0xF800  // Red power bar
#define COLOR_BAR_SPEED     0x07FF  // Cyan speed bar

// ============================================
// System Constants
// ============================================
#define TOUCH_DEBOUNCE_MS   300
#define DISPLAY_UPDATE_MS   100
#define STATUS_READ_MS      500
#define LED_UPDATE_MS       50

// ============================================
// Operating Modes
// ============================================
enum LaserMode {
    MODE_IDLE = 0,      // Em espera, laser desligado
    MODE_READY,         // Pronto para cortar (água OK, tampa fechada)
    MODE_CUTTING,       // Cortando (laser ativo)
    MODE_PAUSED,        // Pausa durante corte
    MODE_ERROR,         // Erro de segurança
    MODE_WARMUP         // Aquecimento
};

// ============================================
// Touch Button IDs
// ============================================
enum TouchButton {
    BTN_NONE = 0,
    BTN_POWER_UP,       // Aumentar potência
    BTN_POWER_DOWN,     // Diminuir potência
    BTN_SPEED_UP,       // Aumentar velocidade
    BTN_SPEED_DOWN,     // Diminuir velocidade
    BTN_LASER_TOGGLE,   // Liga/desliga laser
    BTN_AIR_TOGGLE,     // Liga/desliga ar comprimido
    BTN_MATERIAL_NEXT,  // Próximo preset de material
    BTN_MATERIAL_PREV,  // Preset anterior
    BTN_TEST_PULSE,     // Pulso de teste (100ms)
    BTN_EXHAUST_TOGGLE, // Liga/desliga exaustor
    BTN_ESTOP           // Parada de emergência (software)
};

// ============================================
// Material Cutting Presets
// ============================================
struct CuttingPreset {
    const char* name;       // Nome do material
    uint8_t power;          // Potência (%)
    uint8_t speed;          // Velocidade (mm/s)
    bool airAssist;         // Ar comprimido
    const char* gas;        // Tipo de gás (O2, N2, Ar)
    float thickness;        // Espessura (mm)
};

// Presets para Kindlelaser Max 260W - Corte de Metal
#define PRESET_COUNT 12

static const CuttingPreset CUTTING_PRESETS[PRESET_COUNT] = {
    // Aço Carbono (com O₂)
    {"Aco 0.5mm",   70,  30, true, "O2",  0.5f},
    {"Aco 1.0mm",   85,  20, true, "O2",  1.0f},
    {"Aco 1.5mm",   92,  15, true, "O2",  1.5f},
    {"Aco 2.0mm",   95,  12, true, "O2",  2.0f},
    {"Aco 3.0mm",   99,   8, true, "O2",  3.0f},
    // Inox (com N₂)
    {"Inox 0.5mm",  75,  25, true, "N2",  0.5f},
    {"Inox 1.0mm",  90,  15, true, "N2",  1.0f},
    {"Inox 1.5mm",  95,  10, true, "N2",  1.5f},
    {"Inox 2.0mm",  99,   6, true, "N2",  2.0f},
    // Alumínio (com N₂)
    {"Alum 0.5mm",  80,  20, true, "N2",  0.5f},
    {"Alum 1.0mm",  92,  12, true, "N2",  1.0f},
    {"Alum 1.5mm",  99,   7, true, "N2",  1.5f},
};

// ============================================
// LED Status Modes
// ============================================
enum LEDStatus {
    LED_STATUS_OFF = 0,
    LED_STATUS_IDLE,        // Azul pulsante - em espera
    LED_STATUS_READY,       // Verde fixo - pronto
    LED_STATUS_CUTTING,     // Vermelho pulsante rápido - cortando
    LED_STATUS_WARNING,     // Amarelo piscante - aviso
    LED_STATUS_ERROR,       // Vermelho piscante - erro
    LED_STATUS_COOLDOWN     // Azul gradiente - resfriando
};

#endif
