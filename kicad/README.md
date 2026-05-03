# PCB Level Shifter — ESP32 + SN74HCT245 + WS2812B

Placa de circuito impresso para converter o sinal de 3.3V do ESP32 para 5V compativel com os LEDs WS2812B, usando o CI **SN74HCT245**.

## Esquema do Circuito

```
                         SN74HCT245 (U1)
                    ┌─────────────────────┐
  +5V ──────────────┤ 1  DIR         VCC 20├──── +5V
                    │                      │
  ESP32 GPIO13 ─────┤ 2  A1          B1  18├──[330R]──── WS2812B DIN
                    │ 3  A2          B2  17│
                    │ 4  A3          B3  16│
                    │ 5  A4          B4  15│  (pinos A2-A8 e B2-B8
                    │ 6  A5          B5  14│   disponiveis para
                    │ 7  A6          B6  13│   sinais adicionais)
                    │ 8  A7          B7  12│
                    │ 9  A8          B8  11│
                    │                      │
  GND ──────────────┤10  GND        ~OE 19├──── GND
                    └─────────────────────┘
```

## Por que SN74HCT245?

O ESP32 opera com logica de **3.3V**, mas os LEDs WS2812B precisam de sinais de **5V**.
O SN74HCT245 e um buffer bidirecional com entradas compativeis TTL — aceita **3.3V como nivel HIGH** e produz saida em **5V**.

- **DIR = HIGH (5V)**: Direcao A → B (ESP32 → WS2812B)
- **~OE = LOW (GND)**: Saidas habilitadas
- **VCC = 5V**: Alimentacao do CI
- Canal A1/B1 usado para o sinal de dados

## Conectores

| Conector | Pinos | Funcao |
|----------|-------|--------|
| **J1** (ESP32) | 1: GPIO13, 2: 3.3V, 3: 5V, 4: GND | Conexao com ESP32 DevKit |
| **J2** (WS2812B) | 1: DIN, 2: VCC (5V), 3: GND | Saida para fita LED |
| **J3** (Alimentacao) | 1: +5V, 2: GND | Entrada de alimentacao externa |

## Lista de Materiais (BOM)

| Ref | Componente | Valor | Footprint | Qtd |
|-----|-----------|-------|-----------|-----|
| U1 | SN74HCT245N | - | DIP-20 | 1 |
| R1 | Resistor | 330R | Axial | 1 |
| C1 | Capacitor ceramico | 100nF | Disco 5mm | 1 |
| C2 | Capacitor eletrolitico | 1000uF/10V | Radial 8mm | 1 |
| C3 | Capacitor ceramico | 100nF | Disco 5mm | 1 |
| J1 | Barra de pinos | 1x4 | 2.54mm | 1 |
| J2 | Barra de pinos | 1x3 | 2.54mm | 1 |
| J3 | Barra de pinos | 1x2 | 2.54mm | 1 |

## Dimensoes da Placa

- **Tamanho**: 50mm x 35mm
- **Camadas**: 2 (F.Cu + B.Cu)
- **Furos de montagem**: 4x nos cantos (1.5mm)

## Como Usar

### Requisitos
- [KiCad 10](https://www.kicad.org/) ou superior

### Abrir o Projeto
1. Abra o KiCad
2. **Arquivo → Abrir Projeto**
3. Navegue ate `kicad/esp32_ws2812b_levelshifter/`
4. Selecione `esp32_ws2812b_levelshifter.kicad_pro`

### Gerar Gerber (para fabricacao)
1. Abra o editor de PCB
2. **Arquivo → Plotar**
3. Selecione as camadas necessarias
4. Exporte para o diretorio `gerber/`

## Diagrama de Conexao com o Projeto ESP32 Radio

```
ESP32 DevKit                Level Shifter PCB          WS2812B LED Strip
┌──────────┐               ┌─────────────────┐        ┌──────────────┐
│          │               │ J1          J2  │        │              │
│  GPIO13 ─┼───────────────┤─1  SN74HCT245  1├────────┤─ DIN         │
│    3.3V ─┼───────────────┤─2              2├────────┤─ VCC (5V)    │
│      5V ─┼───────────────┤─3              3├────────┤─ GND         │
│     GND ─┼───────────────┤─4               │        │              │
│          │               │                 │        └──────────────┘
└──────────┘               │ J3 (Fonte 5V)   │
                           │  1: +5V         │
Fonte 5V/2A ───────────────┤  2: GND         │
                           └─────────────────┘
```

## Notas Importantes

1. **Alimentacao**: Use fonte 5V externa com capacidade minima de 2A para 30 LEDs WS2812B
2. **Capacitor C2 (1000uF)**: Posicione proximo ao conector J2 (WS2812B) para filtrar picos
3. **Capacitor C1 (100nF)**: Posicione proximo ao pino VCC do SN74HCT245
4. **Resistor R1 (330R)**: Protege o pino DIN do WS2812B contra reflexoes de sinal
5. **Canais extras**: Pinos A2-A8 / B2-B8 estao disponiveis para uso futuro (ex: controlar multiplas fitas)
6. **GND compartilhado**: ESP32, level shifter e WS2812B devem compartilhar o mesmo GND
