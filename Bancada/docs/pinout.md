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

## 2. Emulador do Pino CP (Control Pilot — IEC 61851-1 Anexo A)

Mapeamento dos 10 relés de 5V (placa de relés com driver NPN S8050 ou módulo de relés comercial) no **Arduino Mega 2560**:

| Identificação do Sinal | Pino Mega 2560 | Tipo de I/O | Função no Firmware / Ensaio de Norma | Estado / Tolerância Emulada |
| :--- | :---: | :---: | :--- | :--- |
| **`PINO_CP_S7`** | **D22** | Saída Digital | Chave $S_7$: Conexão com Terra de Proteção (PE) | Aberto = Estado AX ($12\text{ V}$) / Fechado = Normal |
| **`PINO_CP_S8`** | **D23** | Saída Digital | Chave $S_8$: Ramo de Falha direta ($120\,\Omega$) antes do diodo | Fechado = Estado E ($1\text{ V}$ / Falha de linha) |
| **`PINO_CP_S9_1`** | **D24** | Saída Digital | Chave $S_{9.1}$: Derivação 1 do Estado B (Bypass de $1870\,\Omega$) | Fechado = BX Nominal ($9\text{ V}$, $2740\,\Omega$) |
| **`PINO_CP_S9_2`** | **D25** | Saída Digital | Chave $S_{9.2}$: Derivação 2 do Estado B (Bypass $1870\,\Omega + 920\,\Omega$) | Fechado = BX Limite Inf. ($8\text{ V}$, $1820\,\Omega$) |
| **`PINO_CP_S10_C`** | **D26** | Saída Digital | Chave $S_{10.2}$: Habilitação do barramento do Estado C | Habilita Ramo $S_{11}$ (Carga sem ventilação) |
| **`PINO_CP_S10_D`** | **D27** | Saída Digital | Chave $S_{10.1}$: Habilitação do barramento do Estado D | Habilita Ramo $S_{12}$ (Carga com ventilação) |
| **`PINO_CP_S11_1`** | **D28** | Saída Digital | Chave $S_{11.1}$: Derivação 1 do Estado C (Bypass de $422\,\Omega$) | Fechado = CX Nominal ($6\text{ V}$, $1301\,\Omega \parallel 2740\,\Omega = 882\,\Omega$) |
| **`PINO_CP_S11_2`** | **D29** | Saída Digital | Chave $S_{11.2}$: Derivação 2 do Estado C (Bypass $422\,\Omega + 392\,\Omega$) | Fechado = CX Limite Inf. ($5\text{ V}$, $909\,\Omega \parallel 2740\,\Omega = 682\,\Omega$) |
| **`PINO_CP_S12_1`** | **D30** | Saída Digital | Chave $S_{12.1}$: Derivação 1 do Estado D (Bypass de $178\,\Omega$) | Fechado = DX Nominal ($3\text{ V}$, $270\,\Omega \parallel 2740\,\Omega = 245\,\Omega$) |
| **`PINO_CP_S12_2`** | **D31** | Saída Digital | Chave $S_{12.2}$: Derivação 2 do Estado D (Bypass $178\,\Omega + 130\,\Omega$) | Fechado = DX Limite Inf. ($2\text{ V}$, $140\,\Omega \parallel 2740\,\Omega = 133\,\Omega$) |

*Nota: Em repouso (com os relés de bypass desenergizados), os ramos operam no Limite Superior ($10\text{ V}$ para B, $7\text{ V}$ para C e $4\text{ V}$ para D).*

---

## 3. Parâmetros Elétricos e Segurança

- **Nível Lógico:** 5.0 V (TTL);
- **Lógica de Relés:** Configurada via `NIVEL_RELE_ATIVO` (`HIGH` para driver discreto NPN S8050 / `LOW` para módulos comerciais optoacoplados ativos em nível baixo);
- **Comutação Break-Before-Make:** Assegura que apenas uma chave MOSFET do cabo PP conduza por vez, com intervalo de acomodação de $5\,\mu\text{s}$ para evitar resistências espúrias em paralelo;
- **Resistores de Gate:** Recomendado uso de resistores pull-down de $10\text{ k}\Omega$ nos gates dos MOSFETs e nas bases dos transistores de relé para mantê-los em corte durante o reset/inicialização do microcontrolador.
