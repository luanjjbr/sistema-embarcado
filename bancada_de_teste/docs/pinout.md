# Mapeamento de Hardware e Pinagem da Bancada de Teste

**Projeto:** Bancada de Testes de Estação de Recarga para Veículos Elétricos  
**Normas de Referência:** IEC 61851-1 (Modo 3) e IEC 62196-2 (Tipo 2)  
**Fonte:** Rosa et al. (UFSM) - *Desenvolvimento de uma Estação de Recarga Modo 3 para Veículos Elétricos*

---

## 1. Emulador do Cabo de Recarga (Pino PP - Proximity Pilot)

O pino **PP (Proximity Pilot)** tem a finalidade de informar à Estação de Recarga (EVSE) qual é a capacidade máxima de corrente suportada pelo cabo de carregamento conectado, ou se o cabo foi desconectado.

Na bancada de teste, o pino PP é conectado a um barramento de resistores de precisão comutados por chaves semicondutoras (MOSFETs Canal N / Optoacopladores) para o terra de proteção (**PE**).

### Tabela de Canais e Resistências (IEC 62196-2)

| Chave | Resistor ($R_{PP}$) | Capacidade de Corrente | Pino Arduino Nano | Pino ESP32 DevKit | Função / Diagnóstico |
| :---: | :---: | :---: | :---: | :---: | :--- |
| **0** | Aberto ($\infty$) | 0 A | Nenhum (LOW) | Nenhum (LOW) | **PADRÃO:** Todas as chaves desligadas |
| **S1** | $4700\,\Omega$ | 0 A | **D4** | **GPIO 16** | Desconectado nominal (cabo solto) |
| **S2** | $1500\,\Omega$ | **13 A** | **D5** | **GPIO 17** | Cabo monofásico 13 A |
| **S3** | $680\,\Omega$ | **20 A** | **D6** | **GPIO 18** | Cabo 20 A |
| **S4** | $220\,\Omega$ | **32 A** | **D7** | **GPIO 19** | Cabo 32 A (típico comercial) |
| **S5** | $100\,\Omega$ | **63 A / 70 A** | **D8** | **GPIO 21** | Cabo trifásico de alta potência |
| **S6** | $47\,\Omega$ | 0 A (Falha) | **D9** | **GPIO 22** | Simulação de curto ou falha de isolação |

> [!IMPORTANT]
> **Intertravamento *Break-Before-Make*:**  
> A classe `EmuladorCabo` desliga fisicamente todas as chaves antes de ligar a chave solicitada, impedindo que dois resistores fiquem em paralelo no pino PP.

---

## 2. Emulador do Veículo Elétrico (Pino CP - Control Pilot) [Próxima Etapa]

O pino **CP (Control Pilot)** é a linha de comunicação bidirecional por modulação em largura de pulso (PWM de 1 kHz, $\pm 12\,\text{V}$) entre a estação e o carro.

### Estados do Veículo (IEC 61851-1)

| Estado | Tensão no CP | Circuito no Veículo | Significado |
| :---: | :---: | :---: | :--- |
| **A** | $+12\,\text{V}$ | Aberto | Veículo não conectado |
| **B** | $+9\,\text{V}$ | Resistor $2.74\,\text{k}\Omega$ | Veículo conectado (Standby / Aguardando) |
| **C** | $+6\,\text{V}$ | Diodo + Resistor $882\,\Omega$ ($2.74\,\text{k} \parallel 1.3\,\text{k}$) | Veículo pronto para recarga (sem ventilação) |
| **D** | $+3\,\text{V}$ | Diodo + Resistor $246\,\Omega$ ($2.74\,\text{k} \parallel 270\,\Omega$) | Veículo pronto para recarga (com ventilação) |
| **E / F** | $0\,\text{V}$ ou $-12\,\text{V}$ | Curto para terra ou inversão | Falha na estação ou corte de energia |

---

## 3. Indicadores Visuais da Bancada

| Sinal | Arduino Nano | ESP32 DevKit | Comportamento |
| :---: | :---: | :---: | :--- |
| **LED Heartbeat** | Pino **D13** | **GPIO 2** | Pisca a cada 500 ms via tarefa FreeRTOS (`tarefaHeartbeat`) |

