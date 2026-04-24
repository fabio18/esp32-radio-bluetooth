# Diagrama de Conexões Detalhado

## Lista de Materiais (BOM)

| Qtd | Componente | Observação |
|-----|------------|------------|
| 1 | ESP32 DevKit V1 (30 pinos) | Ou versão 38 pinos |
| 1 | TEA5767 Módulo FM | Com antena incluída |
| 1 | Display TFT ILI9341 2.4" | 240x320, V1.3, SPI |
| 1 | DAC I2S MAX98357A ou PCM5102 | Para saída de áudio BT |
| 1 | Amplificador PAM8403 | Para saída de áudio FM |
| 2 | Alto-falante 3W 4Ω | Estéreo |
| 6 | Push Button 6x6mm | Para controles |
| 1 | Encoder Rotativo KY-040 | Opcional |
| 2 | Resistor 10kΩ | Pull-up para GPIO 34/35 |
| 1 | Protoboard ou PCB | Para montagem |
| - | Fios jumper | Macho-macho e macho-fêmea |
| 1 | Fonte 5V 1A | USB ou externa |

## Pinout ESP32 DevKit

```
                    ┌───────────────────┐
                    │    ESP32 DevKit    │
                    │                   │
              3V3 ──┤ 3V3         VIN ├── 5V
              GND ──┤ GND         GND ├── GND
     Encoder CLK ──┤ GPIO15*    GPIO13├──
    Display CS   ──┤ GPIO15     GPIO12├── Encoder SW
               ──┤ GPIO2(DC)   GPIO14├── Encoder DT
    Display RST ──┤ GPIO4      GPIO27├── Encoder CLK
     I2S BCLK  ──┤ GPIO5      GPIO26├── BTN Preset-
               ──┤ GPIO18(SCK) GPIO25├── BTN Preset+
    Display MOSI──┤ GPIO23     GPIO33├── BTN Mute
    Display MISO──┤ GPIO19     GPIO32├── BTN Seek-
               ──┤ GPIO22(SCL) GPIO35├── BTN Mode (*)
    I2C SCL    ──┤ GPIO22     GPIO34├── BTN Seek+ (*)
    I2C SDA    ──┤ GPIO21     GPIO39├──
    I2S DOUT   ──┤ GPIO16     GPIO36├──
    I2S LRC    ──┤ GPIO17          ├──
                    └───────────────────┘

(*) GPIO 34/35 = input-only, requer pull-up externo 10kΩ
```

## Circuito dos Botões

Cada botão conecta o GPIO ao GND quando pressionado:

```
    3.3V ─── [10kΩ] ─┬─── GPIO (34 ou 35)
                      │
                    [BTN]
                      │
                     GND
```

Para GPIOs com pull-up interno (25, 26, 32, 33):

```
    GPIO ─── [BTN] ─── GND
    (pull-up interno ativado no firmware)
```

## Conexão TEA5767

```
    TEA5767 Module
    ┌─────────────┐
    │ VCC ────────┤──── 3.3V
    │ GND ────────┤──── GND
    │ SDA ────────┤──── GPIO 21 (com pull-up 4.7kΩ para 3.3V)
    │ SCL ────────┤──── GPIO 22 (com pull-up 4.7kΩ para 3.3V)
    │ Audio L ────┤──── Amplificador Left In
    │ Audio R ────┤──── Amplificador Right In
    └─────────────┘
```

## Conexão Display ILI9341

```
    ILI9341 TFT 2.4"
    ┌─────────────┐
    │ VCC ────────┤──── 3.3V
    │ GND ────────┤──── GND
    │ CS  ────────┤──── GPIO 15
    │ RESET ──────┤──── GPIO 4
    │ DC  ────────┤──── GPIO 2
    │ SDI(MOSI)───┤──── GPIO 23
    │ SCK ────────┤──── GPIO 18
    │ LED ────────┤──── 3.3V (sempre ligado)
    │ SDO(MISO)───┤──── GPIO 19
    └─────────────┘
```

## Conexão DAC I2S (para áudio Bluetooth)

### Opção 1: MAX98357A (amplificador I2S com saída de speaker)
```
    MAX98357A
    ┌─────────────┐
    │ VIN ────────┤──── 5V
    │ GND ────────┤──── GND
    │ BCLK ───────┤──── GPIO 5
    │ LRC ────────┤──── GPIO 17
    │ DIN ────────┤──── GPIO 16
    │ GAIN ───────┤──── (não conectar = 9dB)
    │ SD  ────────┤──── (não conectar = ativo)
    │ Speaker+ ───┤──── Alto-falante +
    │ Speaker- ───┤──── Alto-falante -
    └─────────────┘
```

### Opção 2: PCM5102 (DAC I2S com saída de linha)
```
    PCM5102
    ┌─────────────┐
    │ VCC ────────┤──── 3.3V
    │ GND ────────┤──── GND
    │ BCK ────────┤──── GPIO 5
    │ LRCK ───────┤──── GPIO 17
    │ DIN ────────┤──── GPIO 16
    │ SCK ────────┤──── GND
    │ FMT ────────┤──── GND (I2S standard)
    │ XSMT ───────┤──── 3.3V (unmute)
    │ Audio L ────┤──── Para amplificador
    │ Audio R ────┤──── Para amplificador
    └─────────────┘
```

## Dicas de Montagem

1. **Alimentação**: Use capacitor de 100µF na alimentação do ESP32 para estabilidade
2. **I2C Pull-ups**: Adicione resistores de 4.7kΩ pull-up nos pinos SDA e SCL para 3.3V
3. **Antena FM**: Posicione a antena do TEA5767 longe do display e ESP32 para reduzir interferência
4. **Aterramento**: Use um plano de GND comum para todos os componentes
5. **Cabos de áudio**: Use cabo blindado para as conexões de áudio do TEA5767
