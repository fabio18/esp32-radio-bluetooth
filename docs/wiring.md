# Diagrama de Conexões Detalhado

## Lista de Materiais (BOM)

| Qtd | Componente | Observação |
|-----|------------|------------|
| 1 | ESP32 DevKit V1 (30 pinos) | Ou versão 38 pinos |
| 1 | TEA5767 Módulo FM | Com antena incluída |
| 1 | Display TFT ILI9341 2.4" V1.3 | 240x320, SPI, **com touch XPT2046** |
| 1 | Fita LED WS2812B (30 LEDs) | Endereçável, 5V |
| 1 | DAC I2S MAX98357A ou PCM5102 | Para saída de áudio BT |
| 1 | Amplificador PAM8403 | Para saída de áudio FM |
| 2 | Alto-falante 3W 4Ω | Estéreo |
| 2 | Resistor 10kΩ | Divisor de tensão ADC |
| 1 | Resistor 330Ω | Proteção pino de dados WS2812B |
| 1 | Capacitor 100nF | Filtro ADC |
| 1 | Capacitor 1000µF | Proteção alimentação LEDs |
| 1 | Protoboard ou PCB | Para montagem |
| - | Fios jumper | Macho-macho e macho-fêmea |
| 1 | Fonte 5V 2A | USB ou externa (LEDs consomem mais) |

> **Sem botões físicos!** O display ILI9341 2.4" V1.3 já inclui controlador touch XPT2046.

## Pinout ESP32 DevKit

```
                    ┌───────────────────┐
                    │    ESP32 DevKit    │
                    │                   │
              3V3 ──┤ 3V3         VIN ├── 5V
              GND ──┤ GND         GND ├── GND
    Display CS   ──┤ GPIO15     GPIO13├── WS2812B Data
               ──┤ GPIO2(DC)   GPIO12├──
    Display RST ──┤ GPIO4      GPIO14├──
     I2S BCLK  ──┤ GPIO5      GPIO27├── Touch CS (XPT2046)
               ──┤ GPIO18(SCK) GPIO26├──
    Display MOSI──┤ GPIO23     GPIO25├──
    Display MISO──┤ GPIO19     GPIO33├──
               ──┤ GPIO22(SCL) GPIO32├──
    I2C SCL    ──┤ GPIO22     GPIO35├──
    I2C SDA    ──┤ GPIO21     GPIO34├── Touch IRQ (opcional)
    I2S DOUT   ──┤ GPIO16     GPIO39├──
    I2S LRC    ──┤ GPIO17     GPIO36├── Audio ADC In (VP)
                    └───────────────────┘
```

## Conexão TEA5767 (I2C)

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

## Conexão Display ILI9341 + Touch XPT2046 (SPI)

O display ILI9341 2.4" V1.3 já tem o touch XPT2046 integrado.
Ambos compartilham o barramento SPI (MOSI, MISO, SCK) com CS separados.

```
    ILI9341 TFT 2.4" com Touch
    ┌─────────────────┐
    │ VCC ────────────┤──── 3.3V
    │ GND ────────────┤──── GND
    │ CS  ────────────┤──── GPIO 15  (Display CS)
    │ RESET ──────────┤──── GPIO 4
    │ DC  ────────────┤──── GPIO 2
    │ SDI(MOSI) ──────┤──── GPIO 23  (SPI compartilhado)
    │ SCK ────────────┤──── GPIO 18  (SPI compartilhado)
    │ LED ────────────┤──── 3.3V     (sempre ligado)
    │ SDO(MISO) ──────┤──── GPIO 19  (SPI compartilhado)
    │                 │
    │ T_CLK ──────────┤──── GPIO 18  (compartilhado com SCK)
    │ T_CS  ──────────┤──── GPIO 27  (Touch CS)
    │ T_DIN ──────────┤──── GPIO 23  (compartilhado com MOSI)
    │ T_DO  ──────────┤──── GPIO 19  (compartilhado com MISO)
    │ T_IRQ ──────────┤──── GPIO 34  (opcional, input-only)
    └─────────────────┘
```

## Conexão WS2812B LED Strip

```
    WS2812B LED Strip (30 LEDs)
    ┌─────────────────┐
    │                 │
    │ DIN ──[330Ω]───┤──── GPIO 13
    │ VCC ────────────┤──── 5V (fonte externa)
    │ GND ────────────┤──── GND (compartilhado com ESP32)
    │                 │
    └─────────────────┘

    Importante:
    - Capacitor 1000µF entre VCC e GND perto da fita LED
    - Resistor 330Ω no pino DIN para proteção
    - Se usar mais de 10 LEDs, use fonte 5V externa (não alimentar pelo ESP32)
    - Cada LED consome até 60mA no brilho máximo (30 LEDs = até 1.8A)
```

## Entrada de Áudio para LED (Divisor de Tensão)

Para que os LEDs reajam ao som, conecte a saída de áudio ao ADC:

```
    Saída de Áudio
    (do amplificador)
         │
      [10kΩ]  ─── Resistor 1
         │
         ├──────── GPIO 36 (VP / ADC1_CH0)
         │
      [10kΩ]  ─── Resistor 2
         │
      [100nF] ─── Capacitor filtro
         │
        GND
```

> **Nota**: GPIO 36 (VP) é ADC1, sem conflito com Wi-Fi/Bluetooth.
> O divisor de tensão reduz o sinal de áudio para a faixa segura do ADC (0-3.3V).
> O capacitor de 100nF filtra ruído de alta frequência.

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

1. **Alimentação**: Use fonte 5V 2A. LEDs WS2812B podem consumir até 1.8A (30 LEDs)
2. **Capacitor nos LEDs**: 1000µF na alimentação dos LEDs para evitar picos de corrente
3. **I2C Pull-ups**: Adicione resistores de 4.7kΩ pull-up nos pinos SDA e SCL para 3.3V
4. **Antena FM**: Posicione a antena do TEA5767 longe do display e ESP32 para reduzir interferência
5. **Aterramento**: Use um plano de GND comum para todos os componentes
6. **Cabos de áudio**: Use cabo blindado para as conexões de áudio do TEA5767
7. **Resistor 330Ω**: Sempre usar no pino de dados do WS2812B para proteção
8. **Distância dos LEDs**: Se a fita LED estiver longe do ESP32, use cabo curto no pino de dados
