# ESP32 Radio Bluetooth com Alexa — Pioneer Style

Rádio FM com Bluetooth para conectar com a **Amazon Alexa** usando ESP32.
Interface visual estilo **Pioneer automotivo** com controle 100% pela **tela touch**.
**LED WS2812B reativo ao som** com 5 efeitos visuais!

## Componentes

| Componente | Descrição |
|---|---|
| **ESP32 DevKit** | Microcontrolador com Wi-Fi e Bluetooth |
| **TEA5767** | Módulo Rádio FM Estéreo (76-108 MHz) com antena |
| **ILI9341 TFT 2.4"** | Display 240x320 V1.3 **com touch XPT2046** |
| DAC I2S | MAX98357A ou PCM5102 (saída de áudio BT) |
| **WS2812B** | Fita LED endereçável (30 LEDs) reativa ao som |
| Amplificador | PAM8403 ou similar (saída de áudio FM) |
| Alto-falantes | 2x 3W 4Ω (estéreo) |

> **Sem botões físicos!** Tudo é controlado pela tela touch.

## Visual Pioneer

Interface inspirada em rádios automotivos Pioneer/Kenwood:

- Fundo escuro com cores **neon cyan/blue**
- Frequência em **dígitos grandes** com efeito glow
- **Equalizador animado** com barras de espectro coloridas
- Botões touch com **bordas luminosas** estilo automotivo
- Barra de volume com gradiente de cor
- Medidor de sinal estilo LED
- Indicadores STEREO/MONO
- Preset strip com navegação por toque
- **Fita LED WS2812B** reativa ao áudio com 5 efeitos

### Layout da Tela FM
```
┌─────────────────────────────┐
│ ESP32 RADIO          [FM]   │  Header
├─────────────────────────────┤
│ STEREO          SIG ▮▮▮▮▯  │  Status
├─────────────────────────────┤
│                             │
│        101.1                │  Frequência (grande, cyan)
│        FM MHz               │
│                             │
│  ▎▌█▌▎█▌▎▌█▎▌▎█▌▎█        │  EQ Visualizer
├─────────────────────────────┤
│     P3/10  93.7 MHz         │  Preset
├─────────────────────────────┤
│ [<<SEEK] [<P] [P>] [SEEK>>]│  Controles Seek/Preset
│ [VOL -]  [MUTE]   [VOL +]  │  Volume/Mute
│ [LED: VU METER] [BRILHO]   │  LED Controls
│ [═══════ BLUETOOTH ════════]│  Trocar modo
├─────────────────────────────┤
│ ████████████░░░░░░░░░  80   │  Volume slider
└─────────────────────────────┘
```

### Layout da Tela Bluetooth
```
┌─────────────────────────────┐
│ ESP32 RADIO          [BT]   │  Header
├─────────────────────────────┤
│          ╭────╮             │
│          │ BT │             │  Ícone BT com glow
│          ╰────╯             │
│       CONECTADO             │
│         Alexa               │
│      >> TOCANDO >>          │
│  ▎▌█▌▎█▌▎▌█▎▌▎█▌▎█        │  EQ Visualizer
├─────────────────────────────┤
│ [══════ > TOCAR ═══════════]│  Play/Pause
│ [VOL -]  [MUTE]   [VOL +]  │  Volume/Mute
│ [LED: VU METER] [BRILHO]   │  LED Controls
│ [═══════ FM RADIO ═════════]│  Trocar modo
├─────────────────────────────┤
│ ████████████░░░░░░░░░  80   │  Volume slider
└─────────────────────────────┘
```

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

### ILI9341 TFT + Touch → ESP32 (SPI)

```
ILI9341      ESP32         Função
───────      ─────         ──────
MOSI   ───── GPIO 23       SPI Data (compartilhado)
MISO   ───── GPIO 19       SPI Data (compartilhado)
SCK    ───── GPIO 18       SPI Clock (compartilhado)
CS     ───── GPIO 15       Chip Select Display
DC     ───── GPIO  2       Data/Command
RST    ───── GPIO  4       Reset
LED    ───── 3.3V          Backlight
T_CS   ───── GPIO 27       Chip Select Touch (XPT2046)
T_IRQ  ───── GPIO 34       Touch Interrupt (opcional)
VCC    ───── 3.3V
GND    ───── GND
```

> O display e o touch compartilham o barramento SPI (MOSI, MISO, SCK).
> Cada um tem seu próprio CS (Chip Select).

### DAC I2S (Saída de Áudio Bluetooth)

```
DAC I2S      ESP32
───────      ─────
BCLK   ───── GPIO  5
LRC    ───── GPIO 17
DIN    ───── GPIO 16
VCC    ───── 3.3V/5V
GND    ───── GND
```

### WS2812B LED Strip

```
WS2812B      ESP32
───────      ─────
DIN    ───── GPIO 13       Data (com resistor 330Ω)
VCC    ───── 5V            Alimentação (usar fonte externa para >10 LEDs)
GND    ───── GND           Compartilhado com ESP32
```

### Entrada de Áudio para LEDs (Divisor de Tensão)

```
Saída de Áudio (FM ou amplificador)
     │
  [10kΩ]
     │
     ├───── GPIO 36 (VP - ADC)
     │
  [10kΩ]
     │
  [100nF] capacitor para filtrar ruído
     │
    GND
```

> Conecte a saída de áudio (após o amplificador) ao divisor de tensão.
> O capacitor de 100nF filtra ruído de alta frequência.
> GPIO 36 (VP) é ADC1_CH0, sem conflito com Wi-Fi/BT.

### Diagrama Completo

```
                    ┌─────────────────────┐
                    │     ESP32 DevKit     │
                    │                     │
   TEA5767 ────I2C──┤ GPIO21(SDA)         │
   (FM Radio)       │ GPIO22(SCL)         │
                    │                     │
   ILI9341 ───SPI──┤ GPIO23(MOSI)        │── compartilhado
   (Display)        │ GPIO19(MISO)        │── com touch
                    │ GPIO18(SCK)         │── XPT2046
                    │ GPIO15(CS Display)  │
                    │ GPIO2 (DC)          │
                    │ GPIO4 (RST)         │
                    │                     │
   XPT2046 ────────┤ GPIO27(CS Touch)    │
   (Touch)          │ GPIO34(IRQ)         │
                    │                     │
   DAC I2S  ───────┤ GPIO5 (BCLK)       │
   (BT Audio)       │ GPIO17(LRC)         │
                    │ GPIO16(DOUT)        │
                    │                     │
   WS2812B ────────┤ GPIO13(LED Data)    │
   (LED Strip)      │                     │
                    │                     │
   Audio In ───ADC──┤ GPIO36(VP - ADC)    │
   (Divisor)        │                     │
                    └─────────────────────┘
```

---

## Como Compilar e Gravar

### Opção 1: PlatformIO (recomendado)

**Pré-requisitos:** Instale o [PlatformIO](https://platformio.org/install)

```bash
# Compilar
pio run

# Gravar no ESP32
pio run --target upload

# Monitor Serial
pio device monitor
```

> As bibliotecas são instaladas automaticamente pelo PlatformIO.

### Opção 2: Arduino IDE

**Pré-requisitos:**

1. Instale o [Arduino IDE](https://www.arduino.cc/en/software) (1.8.x ou 2.x)
2. Adicione o suporte ESP32:
   - Vá em **Arquivo → Preferências**
   - Em "URLs Adicionais para Gerenciadores de Placas" adicione:
     ```
     https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
     ```
   - Vá em **Ferramentas → Placa → Gerenciador de Placas**
   - Pesquise "ESP32" e instale **esp32 by Espressif Systems**

3. Instale as bibliotecas (**Sketch → Incluir Biblioteca → Gerenciar Bibliotecas**):
   - **TFT_eSPI** by Bodmer
   - **FastLED** by Daniel Garcia
   - **ESP32-A2DP** by Phil Schatzmann

4. **IMPORTANTE** — Configure o TFT_eSPI:
   - Copie o arquivo `arduino/esp32_radio_bluetooth/User_Setup.h`
   - Cole na pasta da biblioteca TFT_eSPI (substituindo o existente):
     - **Windows:** `Documentos/Arduino/libraries/TFT_eSPI/User_Setup.h`
     - **Mac:** `~/Documents/Arduino/libraries/TFT_eSPI/User_Setup.h`
     - **Linux:** `~/Arduino/libraries/TFT_eSPI/User_Setup.h`

5. Abra o arquivo `arduino/esp32_radio_bluetooth/esp32_radio_bluetooth.ino`

6. Configure a placa em **Ferramentas**:
   - **Placa:** ESP32 Dev Module
   - **Partition Scheme:** Huge APP (3MB No OTA/1MB SPIFFS)
   - **Upload Speed:** 921600
   - **Flash Size:** 4MB
   - **Porta:** selecione a porta COM do ESP32

7. Clique em **Upload** (→)

---

## Como Usar

### Primeiro Boot

1. Tela de splash com logo "ESP32 RADIO" estilo Pioneer
2. Inicia automaticamente no **modo FM** (101.1 MHz)
3. Toque na tela para controlar

### Controles Touch — Modo FM

| Botão na tela | Função |
|---|---|
| **<< SEEK** | Buscar estação anterior |
| **SEEK >>** | Buscar próxima estação |
| **<P** | Preset anterior |
| **P>** | Próximo preset |
| **VOL -** | Diminuir volume |
| **VOL +** | Aumentar volume |
| **MUTE** | Ligar/desligar mudo |
| **VU METER** (LED) | Trocar efeito LED: OFF → VU → SPECTRUM → PULSE → RAINBOW → FIRE |
| **BRILHO** | Ciclar brilho LED: 50 → 100 → 150 → 200 → 250 |
| **BLUETOOTH** | Trocar para modo BT |

### Controles Touch — Modo Bluetooth

| Botão na tela | Função |
|---|---|
| **> TOCAR / \|\| PAUSAR** | Play/Pause |
| **VOL -** | Diminuir volume |
| **VOL +** | Aumentar volume |
| **MUTE** | Ligar/desligar mudo |
| **VU METER** (LED) | Trocar efeito LED |
| **BRILHO** | Ciclar brilho LED |
| **FM RADIO** | Trocar para modo FM |

### Conectar com Alexa

1. Toque em **BLUETOOTH** para entrar no modo BT
2. Na tela aparecerá "AGUARDANDO..."
3. No app Alexa:
   - **Dispositivos** → **Echo & Alexa** → seu Echo
   - **Bluetooth** → **Parear novo dispositivo**
   - Selecione **"ESP32 Radio"**
4. Após conectar, a tela mostrará "CONECTADO"
5. Diga: *"Alexa, toque música"* — o áudio sai pelo ESP32!
6. O equalizador animado responde ao áudio
7. A fita LED WS2812B também reage ao som!

### Efeitos LED WS2812B

| Efeito | Descrição |
|---|---|
| **OFF** | LEDs desligados |
| **VU METER** | Barra verde → amarelo → vermelho (tipo medidor de volume) |
| **SPECTRUM** | Espectro de cores que muda com o áudio |
| **PULSE** | Pulso de cor que detecta batidas |
| **RAINBOW** | Arco-íris cuja velocidade e brilho reagem ao som |
| **FIRE** | Efeito fogo modulado pelo áudio |

---

## Calibração do Touch

Se o toque não estiver preciso, ative a calibração:

1. No código `setup()` em `main.cpp`, descomente a linha:
   ```cpp
   display.calibrateTouch();
   ```
2. Grave novamente no ESP32
3. Toque nos 4 cantos quando solicitado
4. Os valores de calibração serão exibidos no Serial Monitor
5. Atualize os valores em `config.h`:
   ```cpp
   #define TOUCH_MIN_X     <valor1>
   #define TOUCH_MAX_X     <valor2>
   #define TOUCH_MIN_Y     <valor3>
   #define TOUCH_MAX_Y     <valor4>
   ```

---

## Estrutura do Projeto

```
esp32-radio-bluetooth/
├── arduino/                    # Versão Arduino IDE
│   └── esp32_radio_bluetooth/
│       ├── esp32_radio_bluetooth.ino  # Sketch completo (arquivo único)
│       └── User_Setup.h               # Config TFT_eSPI (copiar para a lib)
├── platformio.ini          # Configuração PlatformIO + TFT + Touch
├── include/
│   ├── config.h            # Pinos, cores Pioneer, constantes
│   ├── display_ui.h        # Interface Pioneer + Touch
│   ├── bt_audio.h          # Bluetooth A2DP Sink
│   └── led_effects.h       # Efeitos LED WS2812B
├── lib/
│   └── TEA5767/
│       ├── TEA5767.h        # Driver FM
│       └── TEA5767.cpp
├── src/
│   ├── main.cpp            # Firmware principal (touch + LED)
│   ├── display_ui.cpp      # UI Pioneer + EQ Visualizer + Touch
│   ├── bt_audio.cpp        # Bluetooth A2DP
│   └── led_effects.cpp     # Efeitos LED reativos ao som
└── docs/
    └── wiring.md           # Conexões detalhadas
```

## Dependências

| Biblioteca | Versão | Uso |
|---|---|---|
| [TFT_eSPI](https://github.com/Bodmer/TFT_eSPI) | ^2.5.34 | Display ILI9341 + Touch XPT2046 |
| [ESP32-A2DP](https://github.com/pschatzmann/ESP32-A2DP) | v1.8.0 | Bluetooth A2DP Sink |
| [FastLED](https://github.com/FastLED/FastLED) | ^3.6.0 | WS2812B LED strip |

## Customização

### Alterar nome Bluetooth
Em `include/config.h`:
```cpp
#define BT_DEVICE_NAME  "Meu Radio Pioneer"
```

### Alterar presets FM
Em `lib/TEA5767/TEA5767.cpp`, no construtor:
```cpp
float defaultPresets[] = { 89.1, 91.3, 93.7, 96.1, 98.3, 100.9, 101.1, 103.3, 105.1, 107.5 };
```

### Alterar cores
Em `include/config.h`, modifique as constantes `COLOR_*` (formato RGB565).
Cores neon para estilo Pioneer: `COLOR_NEON_CYAN`, `COLOR_NEON_BLUE`, `COLOR_NEON_GREEN`.

### Alterar quantidade de LEDs
Em `include/config.h`:
```cpp
#define LED_COUNT       60   // Número de LEDs na fita
#define LED_PIN         13   // Pino de dados
#define LED_BRIGHTNESS  200  // Brilho inicial (0-255)
```

## Notas Importantes

- **Touch**: O display ILI9341 2.4" V1.3 já inclui o controlador touch XPT2046
- **SPI compartilhado**: Display e touch usam o mesmo barramento SPI com CS separados
- **GPIO 34**: Input-only (usado para IRQ do touch, opcional)
- **Alimentação**: ESP32 + TEA5767 + Display consomem ~300mA. Use fonte de pelo menos 500mA
- **Antena FM**: Posicione longe do display para melhor recepção
- **DAC I2S**: Para modo BT, use MAX98357A (com amplificador) ou PCM5102 (saída de linha)
- **WS2812B**: Alimentar com 5V (fonte externa para >10 LEDs). Usar resistor de 330Ω no pino de dados
- **ADC**: GPIO 36 (VP) usado para captura de áudio. Usar divisor de tensão com 2x 10kΩ + 100nF
- **Capacitor 1000µF**: Recomendado na alimentação dos LEDs WS2812B para evitar picos

## Licença

MIT License
