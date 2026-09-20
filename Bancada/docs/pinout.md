# Mapeamento de Hardware e Pinagem — Projeto Bancada

Este documento detalha o mapeamento de pinos e as conexões elétricas para os microcontroladores suportados no projeto **Bancada** (**Arduino Nano** e **Arduino Mega 2560**).

---

## 1. Tabela de Conexões de Hardware

| Identificação do Sinal | Microcontrolador: Arduino Nano | Microcontrolador: Arduino Mega 2560 | Tipo de I/O | Função no Firmware |
| :--- | :---: | :---: | :---: | :--- |
| **`PINO_LED_STATUS`** | **D13** | **D13** | Saída Digital | LED integrado para monitoramento de atividade (*Heartbeat* via `tarefaBlink`) |
| **`PINO_CABO_BLINK`** | **D4** | **D4** | Saída Digital | Linha de controle/emulação do cabo via classe `EmuladorCabo` (`tarefaCabo`) |
| **UART RX** | **D0 (RX)** | **D0 (RX0)** | Entrada Digital | Recepção de telemetria e comandos seriais (9600 bps) |
| **UART TX** | **D1 (TX)** | **D1 (TX0)** | Saída Digital | Transmissão de telemetria e comandos seriais (9600 bps) |
| **Alimentação 5V** | **5V** | **5V** | Alimentação | Tensão lógica TTL dos microcontroladores |
| **Terra (GND)** | **GND** | **GND** | Referência | Referência elétrica (0V) |

---

## 2. Diagrama Conceitual de Conexões

```text
       +-------------------------------------------------+
       |           Arduino Nano / Arduino Mega           |
       |                                                 |
       |  [ Pino D13 ] --------> LED Onboard (Heartbeat) |
       |                                                 |
       |  [ Pino D4  ] --------> Saída de Controle Cabo  |
       |                                                 |
       |  [ Pino TX  ] --------> Conversor Serial (USB)  |
       |  [ Pino RX  ] <-------- Conversor Serial (USB)  |
       +-------------------------------------------------+
```

---

## 3. Parâmetros Elétricos

- **Nível Lógico:** 5.0 V (TTL).
- **Corrente Máxima por Pino GPIO (AVR):** 20 mA (máximo absoluto: 40 mA por pino).
- **Resistores de Pull:** Configurável externamente conforme a aplicação conectada ao pino D4.

