# Diagrama de Conexões — ESP32 Kindlelaser Max 260W

## Conexão com a PSU do Laser

A PSU (fonte de alimentação) da Kindlelaser Max 260W possui as seguintes entradas de controle:

| Pino PSU | Função | Conexão ESP32 |
|---|---|---|
| **IN** | Controle de potência (0-5V analógico) | GPIO 25 via conversor de nível |
| **L-ON** | Laser Enable (ativo LOW ou HIGH conforme modelo) | GPIO 26 via optoacoplador |
| **G** (GND) | Referência | GND comum |
| **5V** | Saída 5V da PSU (pode alimentar ESP32) | VIN do ESP32 |
| **WP** | Water Protect (entrada para sensor de fluxo) | Conectar sensor diretamente |

### Circuito de Potência (PWM → 0-5V)

```
ESP32 GPIO 25 ──[330Ω]──┬── Conversor de Nível (3.3V → 5V)
                         │   (ex: TXS0108E, ou transistor + pull-up 5V)
                         │
                         └── [Filtro RC: 1kΩ + 100nF] ──→ PSU "IN"
                              (converte PWM em tensão analógica suave)
```

**Alternativa simples (sem conversor):**
```
ESP32 GPIO 25 ──[PWM 20kHz]──[10kΩ]──┬──[100nF]── GND
                                      │
                                      └── LM358 (buffer opamp, Vcc=5V) ──→ PSU "IN"
```

### Circuito de Enable (Optoacoplador)

```
ESP32 GPIO 26 ──[1kΩ]──→ LED do 4N35
                          │
                          GND

                    4N35 Fototransistor:
                    Coletor ──→ PSU "L-ON"
                    Emissor ──→ PSU "G" (GND)
```

> **IMPORTANTE**: O optoacoplador é OBRIGATÓRIO! A PSU do laser tem alta tensão
> (20-40kV no tubo). Sem isolamento, um curto ou pico pode destruir o ESP32.

---

## Sensores de Segurança

### Sensor de Fluxo de Água (YF-S201 ou similar)

```
YF-S201         ESP32
───────         ─────
VCC (vermelho)  5V (ou 3.3V para modelos 3.3V)
GND (preto)     GND
Signal (amarelo) GPIO 33 (com pull-up interno ativado)
```

O sensor gera pulsos quando há fluxo. O firmware verifica se há atividade.
Condição segura: GPIO 33 = LOW (com pull-up, o sensor puxa para LOW quando ativo).

### Chave de Tampa (Microswitch NC)

```
Microswitch NC (normalmente fechado)
───────────────
COM  ──→ GND
NC   ──→ GPIO 32 (INPUT_PULLUP)
```

- Tampa fechada: GPIO 32 = LOW (microswitch conecta COM a NC)
- Tampa aberta: GPIO 32 = HIGH (pull-up sem conexão)

### Botão de Emergência (NC - Normalmente Fechado)

```
E-STOP (botão cogumelo NC)
──────────────────────────
COM  ──→ 3.3V
NC   ──→ GPIO 39 (VN)
         + [10kΩ] pull-down para GND
```

- Normal: GPIO 39 = HIGH (botão NC passa 3.3V)
- Pressionado: GPIO 39 = LOW (circuito aberto, pull-down leva a LOW)

> **Nota**: GPIO 39 é input-only, não tem pull-up interno. Use resistor externo.

### Sensor de Temperatura (NTC 10kΩ)

```
3.3V ──[10kΩ fixo]──┬──[NTC 10kΩ]── GND
                    │
                    └── GPIO 35 (ADC1_CH7)
```

Fórmula de conversão (Beta equation) implementada no firmware:
- Beta: 3950
- Resistência nominal: 10kΩ @ 25°C

---

## Relés de Controle

### Ar Comprimido

```
ESP32 GPIO 14 ──→ Módulo Relé 5V (IN1)
                     │
                     └── Solenoide 24V (ar comprimido)
                         Alimentado por fonte separada 24V
```

### Exaustor

```
ESP32 GPIO 12 ──→ Módulo Relé 5V (IN2)
                     │
                     └── Contator → Motor exaustor 220V
```

> **ATENÇÃO**: Para 220V, use contator adequado e fiação conforme normas elétricas.

---

## Display TFT + Touch (mesmo do projeto original)

| ILI9341 | ESP32 | Função |
|---|---|---|
| MOSI | GPIO 23 | SPI Data |
| MISO | GPIO 19 | SPI Data |
| SCK | GPIO 18 | SPI Clock |
| CS | GPIO 15 | Chip Select Display |
| DC | GPIO 2 | Data/Command |
| RST | GPIO 4 | Reset |
| T_CS | GPIO 27 | Touch Chip Select |
| T_IRQ | GPIO 34 | Touch Interrupt |

---

## WS2812B LED Strip

| WS2812B | ESP32 | Nota |
|---|---|---|
| DIN | GPIO 13 | Com resistor 330Ω em série |
| VCC | 5V | Fonte externa para >10 LEDs |
| GND | GND | Compartilhado |

Capacitor 1000µF na alimentação recomendado.

---

## Alimentação

| Componente | Tensão | Corrente |
|---|---|---|
| ESP32 | 5V (via USB ou VIN) | ~200mA |
| Display ILI9341 | 3.3V (do ESP32) | ~80mA |
| WS2812B (30 LEDs) | 5V (fonte externa) | até 1.8A |
| Módulo relé | 5V | ~100mA |
| Sensor fluxo | 5V ou 3.3V | ~15mA |

**Recomendação**: Fonte 5V 3A para ESP32 + LEDs + relé.
A PSU do laser geralmente fornece 5V/24V para acessórios.

---

## Comunicação UART com Ruida (Opcional)

Se a máquina usar controladora Ruida RDC6445/6442:

```
ESP32 GPIO 17 (TX) ──→ Ruida RX
ESP32 GPIO 16 (RX) ←── Ruida TX
GND ─────────────────── GND (comum)
```

Baud: 115200, 8N1

Permite receber: posição X/Y, status de operação, feedback.
