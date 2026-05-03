# PCB Level Shifter — ESP32 + SN74HCT125 + WS2812B

Placa de circuito impresso para converter o sinal de 3.3V do ESP32 para 5V compativel com os LEDs WS2812B, usando o CI **SN74HCT125** (Quad Buffer/Line Driver com saidas 3-state).

## Esquema do Circuito

```
                         SN74HCT125 (U1)
                         DIP-14
                    ┌──────────────────┐
  GND ──────────────┤ 1  ~1OE    VCC 14├──── +5V
                    │                   │
  ESP32 GPIO13 ─────┤ 2  1A      ~4OE13├──── GND
                    │                   │
             ┌──────┤ 3  1Y       4A  12├
             │      │                   │
  GND ──────────────┤ 4  ~2OE    4Y  11├
             │      │                   │
             │      ┤ 5  2A      ~3OE10├──── GND
             │      │                   │
             │      ┤ 6  2Y       3A   9├
             │      │                   │
             │      ┤ 7  GND      3Y   8├
             │      └──────────────────┘
             │           │  ││  │
             │          GND ││ +5V
             │           100nF
             │
          [62-100R]
             │
        WS2812B DIN
```

## Por que SN74HCT125?

O ESP32 opera com logica de **3.3V**, mas os LEDs WS2812B precisam de sinais de **5V**.
O SN74HCT125 e um quad buffer com entradas compativeis TTL — aceita **3.3V como nivel HIGH** e produz saida em **5V**.

- **~1OE = GND**: Saida do canal 1 habilitada (active low)
- **~2OE, ~3OE, ~4OE = GND**: Todas as saidas habilitadas
- **1A (pino 2)**: Entrada de dados do GPIO13 do ESP32 (3.3V)
- **1Y (pino 3)**: Saida de dados em 5V → resistor → WS2812B DIN
- **VCC (pino 14) = +5V**: Alimentacao do CI
- **GND (pino 7) = GND**

### Vantagens do SN74HCT125

- Mais simples que o SN74HCT245 (14 pinos vs 20 pinos)
- Saidas independentes com controle individual via ~OE
- Buffer unidirecional (ideal para dados do LED — sempre ESP32 → WS2812B)
- Mesmo principio TTL — aceita 3.3V como HIGH na entrada

## Conectores

| Conector | Pinos | Funcao |
|----------|-------|--------|
| **J1** (ESP32) | 1: GPIO13, 2: 3.3V, 3: 5V, 4: GND | Conexao com ESP32 DevKit |
| **J2** (WS2812B) | 1: DIN, 2: VCC (5V), 3: GND | Saida para fita LED |
| **J3** (Alimentacao) | 1: +5V, 2: GND | Entrada de alimentacao externa |

## Lista de Materiais (BOM)

| Ref | Componente | Valor | Footprint | Qtd |
|-----|-----------|-------|-----------|-----|
| U1 | SN74HCT125N | - | DIP-14 (7.62mm) | 1 |
| R1 | Resistor | 62-100R | Axial | 1 |
| C1 | Capacitor ceramico | 100nF | Disco 5mm | 1 |
| C2 | Capacitor eletrolitico | 1000uF/10V | Radial 8mm | 1 |
| J1 | Barra de pinos | 1x4 | 2.54mm | 1 |
| J2 | Barra de pinos | 1x3 | 2.54mm | 1 |
| J3 | Barra de pinos | 1x2 | 2.54mm | 1 |

## Dimensoes da Placa

- **Tamanho**: 40mm x 30mm
- **Camadas**: 2 (F.Cu + B.Cu)
- **Furos de montagem**: 4x nos cantos (M3)

## Como Usar

### Requisitos
- [KiCad 10](https://www.kicad.org/) ou superior

### Abrir o Projeto
1. Abra o KiCad
2. **Arquivo → Abrir Projeto**
3. Navegue ate `kicad/esp32_ws2812b_levelshifter/`
4. Selecione `esp32_ws2812b_levelshifter.kicad_pro`

### Fluxo de Trabalho
1. Abra o esquematico e verifique as conexoes
2. Rode o ERC (Electrical Rules Check)
3. Atribua/verifique os footprints
4. Abra o editor de PCB e posicione os componentes
5. Roteie as trilhas
6. Rode o DRC (Design Rule Check)
7. Gere os arquivos Gerber para fabricacao

## Diagrama de Conexao com o Projeto ESP32 Radio

```
ESP32 DevKit                Level Shifter PCB          WS2812B LED Strip
┌──────────┐               ┌─────────────────┐        ┌──────────────┐
│          │               │ J1          J2  │        │              │
│  GPIO13 ─┼───────────────┤─1 SN74HCT125  1├────────┤─ DIN         │
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
2. **Capacitor C1 (100nF)**: Posicione proximo aos pinos VCC/GND do SN74HCT125
3. **Capacitor C2 (1000uF)**: Posicione proximo ao conector J2 (WS2812B) para filtrar picos
4. **Resistor R1 (62-100R)**: Protege o pino DIN do WS2812B contra reflexoes de sinal
5. **Canais extras**: Pinos 2A/2Y, 3A/3Y, 4A/4Y estao disponiveis para uso futuro
6. **GND compartilhado**: ESP32, level shifter e WS2812B devem compartilhar o mesmo GND
7. **~OE pinos**: Todos conectados ao GND para manter as saidas sempre habilitadas
