# Mapeamento de Hardware e Pinagem — Projeto Bancada

Este documento detalha o mapeamento de pinos e as conexões elétricas para os microcontroladores suportados no projeto **Bancada** (**Arduino Nano** e **Arduino Mega 2560**), em conformidade com as normas **IEC 61851-1** e **IEC 62196-2** (Pino PP).

---

## 1. Tabela de Conexões de Hardware

| Identificação do Sinal | Pino Arduino Nano | Pino Arduino Mega 2560 | Tipo de I/O | Função no Firmware | Resistor / Estado Emulado |
| :--- | :---: | :---: | :---: | :--- | :--- |
| **`PINO_LED_STATUS`** | **D13** | **D13** | Saída Digital | Heartbeat do FreeRTOS (`tarefaBlink`, 500 ms) | LED Onboard |
| **`PINO_CABO_S1`** | **D4** | **D4** | Saída Digital | Chave MOSFET S1: Desconectado Nominal | $4700\,\Omega$ |
| **`PINO_CABO_S2`** | **D5** | **D5** | Saída Digital | Chave MOSFET S2: Cabo de 13 A | $1500\,\Omega$ |
| **`PINO_CABO_S3`** | **D6** | **D6** | Saída Digital | Chave MOSFET S3: Cabo de 20 A | $680\,\Omega$ |
| **`PINO_CABO_S4`** | **D7** | **D7** | Saída Digital | Chave MOSFET S4: Cabo de 32 A | $220\,\Omega$ |
| **`PINO_CABO_S5`** | **D8** | **D8** | Saída Digital | Chave MOSFET S5: Cabo de 63 A / 70 A | $100\,\Omega$ |
| **`PINO_CABO_S6`** | **D9** | **D9** | Saída Digital | Chave MOSFET S6: Simulação de Falha / Curto | $47\,\Omega$ |
| **UART RX** | **D0 (RX)** | **D0 (RX0)** | Entrada Digital | Recepção de comandos CLI (9600 bps, 8-N-1) | Interface Serial |
| **UART TX** | **D1 (TX)** | **D1 (TX0)** | Saída Digital | Transmissão de respostas e telemetria | Interface Serial |
| **Alimentação 5V** | **5V** | **5V** | Alimentação | Alimentação lógica TTL | Fonte do barramento |
| **Terra (GND/PE)**| **GND** | **GND** | Referência | Referência elétrica conectada ao PE | Terra de Proteção |

---

## 2. Parâmetros Elétricos e Segurança

- **Nível Lógico:** 5.0 V (TTL);
- **Comutação Break-Before-Make:** Assegura que apenas uma chave MOSFET conduza por vez, com intervalo de acomodação de $5\,\mu\text{s}$ para evitar resistências em paralelo;
- **Resistores de Gate:** Recomendado uso de resistores pull-down de $10\text{ k}\Omega$ nos gates dos MOSFETs para manter as chaves em corte no reset/inicialização do microcontrolador.
