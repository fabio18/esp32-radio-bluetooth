# ESP32 Kindlelaser Max 260W — Metal Cutting Controller

Painel de controle **touch screen** para máquina de corte a laser CO2 **Kindlelaser Max 130W/260W**.
Interface visual industrial com **presets de corte de metal**, monitoramento de segurança e LEDs de status.

## Funcionalidades

- **Controle de potência** PWM (5-99%) com display em Watts
- **Controle de velocidade** de corte (1-80 mm/s)
- **12 presets de material** (Aço Carbono, Inox, Alumínio) com espessuras
- **Monitoramento de segurança**: fluxo de água, tampa, temperatura, E-stop
- **Pulso de teste** para alinhamento do laser
- **Ar comprimido** e **exaustor** controlados pela tela
- **LED WS2812B** para indicação visual de status da máquina
- **Interface touch** — sem botões físicos

## Componentes

| Componente | Descrição |
|---|---|
| **ESP32 DevKit** | Microcontrolador com Wi-Fi (para monitoramento futuro) |
| **ILI9341 TFT 2.4"** | Display 240x320 **com touch XPT2046** |
| **WS2812B** | Fita LED (30 LEDs) para status da máquina |
| **Optoacoplador** | 4N35 ou PC817 — isolamento ESP32 ↔ PSU laser |
| **Conversor de nível** | 3.3V → 5V para sinal PWM |
| **Relé 5V (2ch)** | Para ar comprimido e exaustor |
| **Sensor de fluxo** | Fluxo de água (tipo YF-S201) |
| **NTC 10kΩ** | Sensor de temperatura da água |
| **Botão E-STOP** | Emergência (NC - normalmente fechado) |
| **Chave de tampa** | Microswitch na porta da máquina |

## Layout da Tela Touch

```
┌─────────────────────────────┐
│ KINDLELASER 260W    [STATUS]│  Header
├─────────────────────────────┤
│ MAT: Aco 2.0mm        [O2] │  Preset de material
├─────────────────────────────┤
│   POTENCIA: 95%  (247W)    │  Potência (grande)
│   VELOCIDADE: 12 mm/s      │  Velocidade
├─────────────────────────────┤
│ [POT-] [POT+] [SPD-] [SPD+]│  Controles pot/velocidade
│ [<MAT]  [PULSO]  [MAT>]   │  Material presets
│ [AR]  [EXAUST]  [E-STOP]  │  Auxiliares
│ [═══ LASER ON/OFF ════════]│  Botão principal
├─────────────────────────────┤
│ ████████████████░░░░  95%  │  Barra de potência
├─────────────────────────────┤
│ H2O:OK LID:OK 25°C SEGURO │  Status de segurança
└─────────────────────────────┘
```

## Diagrama de Conexões

### ESP32 → PSU Laser (Kindlelaser Max 260W)

```
ESP32 GPIO 25 (PWM) ──[330Ω]──→ Conversor 3.3V→5V ──→ PSU "IN" (0-5V potência)
ESP32 GPIO 26       ──[1kΩ]──→ Optoacoplador 4N35 ──→ PSU "L-ON" (Laser Enable)
                                 (isolamento galvânico obrigatório!)
```

### ESP32 → Relés de Controle

```
ESP32 GPIO 14  ──→ Relé CH1 ──→ Solenoide ar comprimido (24V)
ESP32 GPIO 12  ──→ Relé CH2 ──→ Exaustor/ventilação (220V)
```

### ESP32 ← Sensores de Segurança

```
Sensor de Fluxo (YF-S201)  ──→ GPIO 33 (INPUT_PULLUP)
Chave de Tampa (NC)         ──→ GPIO 32 (INPUT_PULLUP)
Botão E-STOP (NC)           ──→ GPIO 39 (VN - INPUT)
NTC 10kΩ (divisor)          ──→ GPIO 35 (ADC)
```

### Circuito NTC Temperatura

```
3.3V ──[10kΩ]──┬──[NTC 10kΩ]── GND
               │
               └── GPIO 35 (ADC)
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
T_CS   ───── GPIO 27       Chip Select Touch
T_IRQ  ───── GPIO 34       Touch Interrupt (opcional)
VCC    ───── 3.3V
GND    ───── GND
```

### WS2812B LED Strip

```
WS2812B      ESP32
───────      ─────
DIN    ───── GPIO 13       Data (com resistor 330Ω)
VCC    ───── 5V            Fonte externa
GND    ───── GND           Compartilhado
```

### Diagrama Completo

```
                    ┌─────────────────────┐
                    │     ESP32 DevKit     │
                    │                     │
  PSU Laser ──PWM──┤ GPIO25 (PWM)        │── 20kHz → 0-5V potência
  (Kindlelaser)     │ GPIO26 (ENABLE)     │── Optoacoplador → L-ON
                    │                     │
  ILI9341 ───SPI──┤ GPIO23 (MOSI)       │── compartilhado
  (Display)        │ GPIO19 (MISO)       │── com touch
                   │ GPIO18 (SCK)        │── XPT2046
                   │ GPIO15 (CS Display) │
                   │ GPIO2  (DC)         │
                   │ GPIO4  (RST)        │
                   │                     │
  XPT2046 ────────┤ GPIO27 (CS Touch)   │
  (Touch)          │                     │
                   │                     │
  Ar Comprimido ──┤ GPIO14 (Relé)       │── Solenoide 24V
  Exaustor ───────┤ GPIO12 (Relé)       │── Ventilação 220V
                   │                     │
  WS2812B ────────┤ GPIO13 (LED Data)   │
  (Status LEDs)    │                     │
                   │                     │
  Sensor Fluxo  ──┤ GPIO33 (INPUT)      │── Água OK?
  Chave Tampa  ───┤ GPIO32 (INPUT)      │── Tampa fechada?
  E-STOP  ────────┤ GPIO39 (INPUT)      │── Emergência?
  NTC Temp  ──ADC─┤ GPIO35 (ADC)        │── Temperatura água
                   │                     │
                   └─────────────────────┘
```

---

## Presets de Corte de Metal (Kindlelaser 260W)

| # | Material | Espessura | Potência | Velocidade | Gás |
|---|---|---|---|---|---|
| 1 | Aço Carbono | 0.5mm | 70% | 30 mm/s | O₂ |
| 2 | Aço Carbono | 1.0mm | 85% | 20 mm/s | O₂ |
| 3 | Aço Carbono | 1.5mm | 92% | 15 mm/s | O₂ |
| 4 | Aço Carbono | 2.0mm | 95% | 12 mm/s | O₂ |
| 5 | Aço Carbono | 3.0mm | 99% | 8 mm/s | O₂ |
| 6 | Inox | 0.5mm | 75% | 25 mm/s | N₂ |
| 7 | Inox | 1.0mm | 90% | 15 mm/s | N₂ |
| 8 | Inox | 1.5mm | 95% | 10 mm/s | N₂ |
| 9 | Inox | 2.0mm | 99% | 6 mm/s | N₂ |
| 10 | Alumínio | 0.5mm | 80% | 20 mm/s | N₂ |
| 11 | Alumínio | 1.0mm | 92% | 12 mm/s | N₂ |
| 12 | Alumínio | 1.5mm | 99% | 7 mm/s | N₂ |

> **Nota:** Estes valores são pontos de partida. Ajuste conforme o estado do tubo, lente e condições do material. A Kindlelaser 130W corta até ~2mm de aço; a 260W até ~3mm.

---

## Sistema de Segurança

O sistema implementa **4 camadas de segurança**:

| Sensor | Condição para operar | Ação se falhar |
|---|---|---|
| **Fluxo de água** | Sensor detecta fluxo (LOW) | Desliga laser imediatamente |
| **Tampa** | Microswitch fechado (LOW) | Desliga laser imediatamente |
| **E-STOP** | Botão não pressionado (HIGH) | Desliga TUDO (emergency stop) |
| **Temperatura** | < 35°C | Desliga laser + aviso |

Adicionalmente:
- **Tempo máximo de corte contínuo**: 5 minutos → pausa obrigatória
- **Potência máxima limitada**: 99% (protege vida útil do tubo)
- **Ar comprimido obrigatório**: não pode ser desligado durante corte
- **Exaustor automático**: liga junto com o laser

---

## Indicação LED (WS2812B)

| Status | Efeito LED | Significado |
|---|---|---|
| **IDLE** | Azul pulsante (respiração) | Máquina em espera |
| **PRONTO** | Verde fixo | Segurança OK, pronto para cortar |
| **CORTANDO** | Vermelho pulsante rápido | Laser ativo! |
| **AVISO** | Amarelo piscante | Temperatura alta ou pausa |
| **ERRO** | Vermelho piscante | Falha de segurança |
| **RESFRIANDO** | Azul gradiente | Aguardando resfriamento |

---

## Como Compilar e Gravar

### PlatformIO (recomendado)

```bash
# Compilar
pio run

# Gravar no ESP32
pio run --target upload

# Monitor Serial
pio device monitor
```

### Arduino IDE

1. Instale o suporte ESP32 (URL: `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`)
2. Instale bibliotecas: **TFT_eSPI**, **FastLED**
3. Copie `arduino/esp32_radio_bluetooth/User_Setup.h` para a pasta do TFT_eSPI
4. Configure: ESP32 Dev Module, Huge APP (3MB), 921600 baud
5. Upload

---

## Como Usar

### Primeiro Boot

1. Tela splash "KINDLELASER MAX 260W"
2. Verifica condições de segurança
3. Se tudo OK → status "PRONTO" (LED verde)

### Controles Touch

| Botão | Função |
|---|---|
| **POT-** / **POT+** | Diminuir/aumentar potência (±5%) |
| **SPD-** / **SPD+** | Diminuir/aumentar velocidade (±1 mm/s) |
| **<MAT** / **MAT>** | Trocar preset de material |
| **PULSO** | Disparo de teste (100ms) para alinhamento |
| **AR** | Liga/desliga ar comprimido |
| **EXAUST** | Liga/desliga exaustor |
| **E-STOP** | Parada de emergência (software) |
| **LASER ON/OFF** | Liga/desliga o laser |

### Sequência de Operação

1. Ligue a máquina → ESP32 faz checagem de segurança
2. Aguarde LED verde (PRONTO)
3. Selecione o material com **<MAT / MAT>**
4. Ajuste potência e velocidade se necessário
5. Use **PULSO** para verificar alinhamento
6. Toque **LASER ON/OFF** para iniciar corte
7. LED fica vermelho pulsante durante corte
8. Toque novamente para desligar

---

## Estrutura do Projeto

```
esp32-radio-bluetooth/
├── platformio.ini              # Configuração PlatformIO
├── include/
│   ├── config.h                # Pinos, presets, segurança, cores
│   ├── display_ui.h            # Interface touch industrial
│   ├── laser_control.h         # Controle do laser + segurança
│   └── led_effects.h           # LED status strip
├── src/
│   ├── main.cpp                # Firmware principal
│   ├── display_ui.cpp          # UI industrial + touch
│   ├── laser_control.cpp       # PWM + safety + presets
│   └── led_effects.cpp         # Efeitos LED de status
├── docs/
│   └── wiring.md               # Conexões detalhadas
└── arduino/
    └── esp32_radio_bluetooth/
        └── User_Setup.h        # Config TFT_eSPI
```

## Dependências

| Biblioteca | Versão | Uso |
|---|---|---|
| [TFT_eSPI](https://github.com/Bodmer/TFT_eSPI) | ^2.5.34 | Display ILI9341 + Touch XPT2046 |
| [FastLED](https://github.com/FastLED/FastLED) | ^3.6.0 | WS2812B LED status strip |

## Customização

### Alterar presets de corte
Em `include/config.h`, modifique o array `CUTTING_PRESETS[]`:
```cpp
{"Aco 4.0mm", 99, 5, true, "O2", 4.0f},
```

### Alterar limites de segurança
```cpp
#define WATER_TEMP_MAX      35.0f   // Temperatura máxima (°C)
#define MAX_CONTINUOUS_CUT  300000  // Tempo máximo corte (ms)
#define POWER_MAX           99      // Potência máxima (%)
```

### Alterar PWM
```cpp
#define LASER_PWM_FREQ      20000   // Frequência (Hz)
#define LASER_PWM_RESOLUTION 12     // Resolução (bits)
```

---

## ⚠️ AVISOS DE SEGURANÇA

1. **SEMPRE use óculos de proteção** adequados para CO2 (OD5+ @ 10.6µm)
2. **NUNCA opere sem fluxo de água** — o tubo CO2 queima em segundos
3. **Use optoacopladores** entre ESP32 e PSU laser (isolamento galvânico)
4. **O botão E-STOP físico deve funcionar INDEPENDENTE do ESP32** — cabeie diretamente na PSU como segurança adicional
5. **Ventilação obrigatória** — fumos metálicos são tóxicos
6. **Este painel é AUXILIAR** — não substitui o controlador principal (Ruida) para movimentação dos eixos

---

## Comunicação com Ruida (Opcional)

O ESP32 pode se comunicar via UART com a controladora Ruida RDC6445/6442:
- GPIO 16 (RX) ← Ruida TX
- GPIO 17 (TX) → Ruida RX
- Baud: 115200

Isso permite receber feedback de posição e status da controladora principal.

## Licença

MIT License
