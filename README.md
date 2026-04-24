# ESP32 Radio Bluetooth com Alexa

Rádio FM com Bluetooth para conectar com a **Amazon Alexa** usando ESP32.

## Componentes

| Componente | Descrição |
|---|---|
| **ESP32 DevKit** | Microcontrolador com Wi-Fi e Bluetooth |
| **TEA5767** | Módulo Rádio FM Estéreo (76-108 MHz) com antena |
| **ILI9341 TFT 2.4"** | Display 240x320 V1.3 |
| Amplificador de áudio | PAM8403 ou similar (para saída de som) |
| Alto-falantes | 2x 3W 4Ω (estéreo) ou 1x (mono) |
| Botões | 6x push buttons (ou encoder rotativo) |

## Funcionalidades

- **Modo FM**: Sintoniza estações FM (76-108 MHz) com busca automática e presets
- **Modo Bluetooth**: Recebe áudio da Alexa via Bluetooth A2DP
- **Interface gráfica**: Display TFT colorido com informações em tempo real
- **Controles**: Botões físicos para seek, preset, volume, mute e troca de modo
- **Encoder rotativo**: Sintonia fina em FM / ajuste de volume em BT (opcional)

---

## Diagrama de Conexões

### TEA5767 → ESP32 (I2C)

```
TEA5767      ESP32
───────      ─────
SDA    ───── GPIO 21
SCL    ───── GPIO 22
VCC    ───── 3.3V
GND    ───── GND
```

### ILI9341 TFT → ESP32 (SPI)

```
ILI9341      ESP32
───────      ─────
MOSI   ───── GPIO 23
MISO   ───── GPIO 19
SCK    ───── GPIO 18
CS     ───── GPIO 15
DC     ───── GPIO  2
RST    ───── GPIO  4
LED    ───── 3.3V (ou GPIO para controle de brilho)
VCC    ───── 3.3V
GND    ───── GND
```

### Botões → ESP32 (Pull-up interno)

```
Botão            ESP32         Função
─────            ─────         ──────
BTN_MODE    ──── GPIO 35      Alternar FM / Bluetooth
BTN_SEEK+   ──── GPIO 34      Buscar estação (FM) / Vol+ (BT)
BTN_SEEK-   ──── GPIO 32      Buscar estação (FM) / Vol- (BT)
BTN_MUTE    ──── GPIO 33      Mudo (ambos modos)
BTN_PRESET+ ──── GPIO 25      Próximo preset (FM)
BTN_PRESET- ──── GPIO 26      Preset anterior (FM)
```

> **Nota:** GPIO 34 e 35 não possuem pull-up interno no ESP32.
> Use resistores de 10kΩ pull-up externos nesses pinos.

### Encoder Rotativo (Opcional)

```
Encoder      ESP32
───────      ─────
CLK    ───── GPIO 27
DT     ───── GPIO 14
SW     ───── GPIO 12
VCC    ───── 3.3V
GND    ───── GND
```

### Saída de Áudio I2S (DAC externo)

Para Bluetooth, o áudio é transmitido via I2S. Use um DAC como MAX98357A ou PCM5102:

```
DAC I2S      ESP32
───────      ─────
BCLK   ───── GPIO  5
LRC    ───── GPIO 17
DIN    ───── GPIO 16
VCC    ───── 3.3V/5V
GND    ───── GND
```

### Saída de Áudio FM (TEA5767)

O TEA5767 tem saídas analógicas de áudio. Conecte a um amplificador:

```
TEA5767         Amplificador (PAM8403)
───────         ──────────────────────
Audio L  ─────  Left Input
Audio R  ─────  Right Input
GND      ─────  GND
```

---

## Diagrama Completo

```
                    ┌─────────────────────┐
                    │     ESP32 DevKit     │
                    │                     │
   TEA5767 ────I2C──┤ GPIO21(SDA)         │
   (FM Radio)       │ GPIO22(SCL)         │
                    │                     │
   ILI9341 ───SPI──┤ GPIO23(MOSI)        │
   (Display)        │ GPIO19(MISO)        │
                    │ GPIO18(SCK)         │
                    │ GPIO15(CS)          │
                    │ GPIO2 (DC)          │
                    │ GPIO4 (RST)         │
                    │                     │
   Buttons ────────┤ GPIO35(MODE)        │
                    │ GPIO34(SEEK+)       │
                    │ GPIO32(SEEK-)       │
                    │ GPIO33(MUTE)        │
                    │ GPIO25(PRESET+)     │
                    │ GPIO26(PRESET-)     │
                    │                     │
   Encoder ────────┤ GPIO27(CLK)         │
   (Opcional)       │ GPIO14(DT)          │
                    │ GPIO12(SW)          │
                    │                     │
   DAC I2S  ───────┤ GPIO5 (BCLK)       │──── Amplificador
   (BT Audio)       │ GPIO17(LRC)         │──── Alto-falantes
                    │ GPIO16(DOUT)        │
                    └─────────────────────┘
```

---

## Como Compilar e Gravar

### Pré-requisitos

1. Instale o [PlatformIO](https://platformio.org/install)
2. Clone este repositório

### Compilar

```bash
pio run
```

### Gravar no ESP32

```bash
pio run --target upload
```

### Monitor Serial

```bash
pio device monitor
```

---

## Como Usar

### Primeiro Boot

1. Ao ligar, a tela de splash é exibida
2. O rádio inicia no **modo FM** na frequência padrão (101.1 MHz)
3. Use os botões para controlar

### Controles - Modo FM

| Botão | Função |
|---|---|
| **MODE** | Alternar para Bluetooth |
| **SEEK+** | Buscar próxima estação |
| **SEEK-** | Buscar estação anterior |
| **MUTE** | Ligar/desligar mudo |
| **PRESET+** | Próximo preset salvo |
| **PRESET-** | Preset anterior |
| **Encoder** | Sintonia fina (±0.1 MHz) |

### Controles - Modo Bluetooth

| Botão | Função |
|---|---|
| **MODE** | Alternar para FM |
| **SEEK+** | Aumentar volume |
| **SEEK-** | Diminuir volume |
| **MUTE** | Ligar/desligar mudo |
| **Encoder** | Ajustar volume |

### Conectar com Alexa

1. Pressione **MODE** para entrar no modo Bluetooth
2. No display aparecerá "Aguardando..."
3. No app Alexa (celular):
   - Vá em **Dispositivos** → **Echo & Alexa**
   - Selecione seu dispositivo Echo
   - **Configurações Bluetooth** → **Parear novo dispositivo**
   - Selecione **"ESP32 Radio"**
4. Após conectar, o display mostrará "Conectado"
5. Diga: *"Alexa, toque música"* — o áudio sairá pelo ESP32!

---

## Estrutura do Projeto

```
esp32-radio-bluetooth/
├── platformio.ini          # Configuração PlatformIO
├── include/
│   ├── config.h            # Definições de pinos e constantes
│   ├── display_ui.h        # Interface gráfica do display
│   └── bt_audio.h          # Módulo Bluetooth A2DP
├── lib/
│   └── TEA5767/
│       ├── TEA5767.h        # Driver do módulo FM
│       └── TEA5767.cpp
├── src/
│   ├── main.cpp            # Firmware principal
│   ├── display_ui.cpp      # Implementação da UI
│   └── bt_audio.cpp        # Implementação do Bluetooth
└── docs/
    └── wiring.md           # Diagrama de conexões detalhado
```

## Dependências

| Biblioteca | Versão | Uso |
|---|---|---|
| [TFT_eSPI](https://github.com/Bodmer/TFT_eSPI) | ^2.5.34 | Display ILI9341 |
| [ESP32-A2DP](https://github.com/pschatzmann/ESP32-A2DP) | v1.8.0 | Bluetooth A2DP Sink |

## Customização

### Alterar nome Bluetooth

Em `include/config.h`:
```cpp
#define BT_DEVICE_NAME  "Meu Radio"
```

### Alterar presets FM padrão

Em `lib/TEA5767/TEA5767.cpp`, no construtor:
```cpp
float defaultPresets[] = {
    89.1, 91.3, 93.7, 96.1, 98.3, 100.9, 101.1, 103.3, 105.1, 107.5
};
```

### Alterar cores da interface

Em `include/config.h`, modifique as constantes `COLOR_*` (formato RGB565).

### Alterar pinos

Todos os pinos são configuráveis em `include/config.h`.

## Notas Importantes

- **GPIO 34, 35**: Apenas entrada (input-only), sem pull-up interno. Use resistores de 10kΩ pull-up externos.
- **GPIO 2**: Conectado ao LED onboard em alguns DevKits. Funciona normalmente como TFT_DC.
- **Alimentação**: O ESP32 + TEA5767 + Display consomem ~300mA. Use fonte de pelo menos 500mA.
- **Antena FM**: A qualidade da recepção depende muito da antena. Use a antena incluída com o TEA5767.
- **DAC I2S**: Para o modo Bluetooth, é recomendado usar um DAC I2S externo (MAX98357A ou PCM5102) para melhor qualidade de áudio.

## Licença

MIT License
