// ====================================================
// TFT_eSPI User_Setup.h para ESP32 Radio Pioneer
// ====================================================
// COPIE este arquivo para a pasta da biblioteca TFT_eSPI:
//   Arduino/libraries/TFT_eSPI/User_Setup.h
// (substitua o arquivo existente)
// ====================================================

// Driver do display
#define ILI9341_DRIVER

// Resolução
#define TFT_WIDTH  240
#define TFT_HEIGHT 320

// Pinos do Display (SPI)
#define TFT_MOSI  23
#define TFT_MISO  19
#define TFT_SCLK  18
#define TFT_CS    15
#define TFT_DC     2
#define TFT_RST    4

// Pinos do Touch (XPT2046) - compartilha SPI
#define TOUCH_CS  27

// Fontes
#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_FONT4
#define LOAD_FONT6
#define LOAD_FONT7
#define LOAD_FONT8
#define LOAD_GFXFF

#define SMOOTH_FONT

// SPI Frequency
#define SPI_FREQUENCY       40000000
#define SPI_READ_FREQUENCY  20000000
#define SPI_TOUCH_FREQUENCY  2500000
