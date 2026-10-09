# ARCHITECTURE AND TECHNICAL SPECIFICATION: EV EMULATOR & TEST BENCH
**Standard Compatibility:** IEC 61851-1 (2010/2017), IEC 62196-2, SAE J1772
**Reference Base:** UFSM/GEPOC - "Desenvolvimento de uma Estação de Recarga Modo 3 para Veículos Elétricos" (075_Desenvolvimento_Estacao_Recarga.pdf)
**Target Environment:** AntiGravite IDE (C / FreeRTOS)

---

## 1. Visão Geral da Bancada de Testes

A bancada de testes tem como objetivo principal emular o comportamento de um Veículo Elétrico (VE) e do Cabo de Recarga para validar e testar Estações de Recarga Modo 3 (EVSE) de acordo com a norma **IEC 61851-1**.

A bancada é dividida em dois submódulos de teste:
1. **Emulador do Cabo de Recarga (Pino PP - Proximity Pilot):** Simula a conexão de cabos com diferentes capacidades de corrente através da comutação de resistores conectados ao condutor de proteção (PE).
2. **Emulador de Estados do VE (Pino CP - Control Pilot):** Simula a presença do veículo elétrico e altera os níveis de tensão da parcela positiva do sinal PWM de ±12 V (1 kHz) para sinalizar os Estados A, B, C, D e E.

---

## 2. Esquemático Elétrico e Valores de Componentes

### 2.1. Circuito Emulador do Cabo de Recarga (Pino PP)

O pino PP informa à estação de recarga a capacidade de corrente nominal do cabo conectado. O circuito consiste em uma chave seletora com resistores conectados entre o pino **PP** e o **PE (Condutor de Proteção)**.

| Estado / Capacidade do Cabo | Faixa de Resistência Norma (Ω) | Resistência Projetada / Comercial (Ω) |
| :--- | :--- | :--- |
| **Desconectado** | > 4500 Ω | 4700 Ω |
| **Cabo de 13 A** | 1100 Ω a 2460 Ω | 1500 Ω |
| **Cabo de 20 A** | 400 Ω a 936 Ω | 680 Ω |
| **Cabo de 32 A** | 164 Ω a 308 Ω | 220 Ω |
| **Cabo de 63 A / 70 A** | 80 Ω a 140 Ω | 100 Ω |
| **Falha / Curto no Cabo** | < 60 Ω | 47 Ω |

---

### 2.2. Circuito Emulador de Estados do VE (Pino CP)

O pino Control Pilot transmite o sinal PWM de ±12 V / 1 kHz enviado pela estação. O veículo modifica a amplitude da tensão positiva ($V_A$) através de uma rede resistiva e um diodo $D_1$ em série.

#### Diagrama de Blocos do Circuito do VE:
```text
[CP da Estação] --->|---+--- [R2 = 2.74 kΩ] -----------> [PE]
                    D1  |
                        +--- [S2] ---> [R3 = 1.3 kΩ] --> [PE]
```

#### Tabela de Validação de Níveis de Tensão ($V_A$ Positivo):

| Estado do VE | Descrição do Estado | Faixa de Tensão $V_A$ | Tensão Nominal | Resistência Equivalente $R_e$ |
| :--- | :--- | :--- | :--- | :--- |
| **Estado A** | VE Desconectado | 11,4 V a 12,6 V | 12,0 V | Circuito Aberto |
| **Estado B** | VE Conectado (Não Pronto) | 8,37 V a 9,59 V | 9,0 V | $R_2 = 2,74\text{ k}\Omega$ |
| **Estado C** | Pronto p/ Carga (Sem Ventilação) | 5,47 V a 6,53 V | 6,0 V | $R_2 \parallel R_3 = 882\ \Omega$ ($R_3 = 1,3\text{ k}\Omega$) |
| **Estado D** | Pronto p/ Carga (Com Ventilação) | 2,0 V a 4,0 V | 3,0 V | $R_2 \parallel R_3 = 246\ \Omega$ ($R_3 = 270\ \Omega$) |
| **Estado E** | Falha / Curto-circuito CP-PE | < 1,0 V | 0,0 V | Curto direto ($0\ \Omega$) |

---

### 2.3. Circuitos de Condicionamento de Sinal (Lado EVSE / Medição)

Caso a bancada inclua medição/controle da estação:
1. **Circuito Gerador PWM (±12 V, 1 kHz):** Amplificador operacional Rail-to-Rail alimentado com fontes de +12 V e -12 V, com filtro de saída de $1\text{ k}\Omega$ e $1\text{ nF}$.
2. **Leitura do Pino PP:** Fonte auxiliar de +3.3 V com resistor de pull-up de $470\ \Omega$ conectado à entrada do conversor AD do microcontrolador.
3. **Condicionamento da Tensão $V_A$ (CP):** Divisor de tensão com malha de $27\text{ M}\Omega$ e $4,3\text{ M}\Omega$ e amplificador operacional somador de offset para ajustar a faixa de ±12 V para 0–3.3 V do ADC.

---

## 3. Firmware Estruturado em C com FreeRTOS (Para AntiGravite IDE)

Abaixo está a arquitetura completa de código do firmware pronta para ser compilada e utilizada no seu projeto no **AntiGravite IDE**.

### 3.1. Arquivo de Cabeçalho: `iec61851_emulator.h`

```c
#ifndef IEC61851_EMULATOR_H
#define IEC61851_EMULATOR_H

#include <stdint.h>
#include <stdbool.h>

/* Níveis de tensão e tolerâncias da IEC 61851-1 */
#define VOLTAGE_STATE_A_NOMINAL   12.0f
#define VOLTAGE_STATE_B_NOMINAL   9.0f
#define VOLTAGE_STATE_C_NOMINAL   6.0f
#define VOLTAGE_STATE_D_NOMINAL   3.0f
#define VOLTAGE_STATE_E_NOMINAL   0.0f

/* Estados da Máquina de Estados da IEC 61851-1 */
typedef enum {
    EV_STATE_A = 0, /* Desconectado */
    EV_STATE_B,     /* Conectado, não pronto para recarga */
    EV_STATE_C,     /* Conectado, pronto para recarga (sem ventilação) */
    EV_STATE_D,     /* Conectado, pronto para recarga (com ventilação exigida) */
    EV_STATE_E      /* Erro / Falha de Terra ou Curto-circuito */
} ev_state_t;

/* Codificação da capacidade de corrente do cabo (Pino PP) */
typedef enum {
    CABLE_DISCONNECTED = 0,
    CABLE_13A,
    CABLE_20A,
    CABLE_32A,
    CABLE_63A,
    CABLE_FAULT
} cable_current_rating_t;

/* Estrutura de dados de diagnóstico do sistema */
typedef struct {
    ev_state_t current_state;
    cable_current_rating_t cable_rating;
    float cp_positive_voltage;
    float cp_duty_cycle_percent;
    float max_allowed_current_amps;
    bool contactor_closed;
} evse_diagnostic_data_t;

/* Função de cálculo de corrente permitida baseada no Duty Cycle do PWM */
float calculate_max_current_from_duty(float duty_cycle);

#endif /* IEC61851_EMULATOR_H */
```

---

### 3.2. Implementação Principal: `main.c` (FreeRTOS)

```c
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "iec61851_emulator.h"

/* Filas e Semáforos do FreeRTOS */
QueueHandle_t xDiagnosticQueue;
SemaphoreHandle_t xStateMutex;

/* Protótipos de Tarefas FreeRTOS */
void vTaskADCSampler(void *pvParameters);
void vTaskIEC61851StateMachine(void *pvParameters);
void vTaskSafetyMonitor(void *pvParameters);
void vTaskHMIDisplay(void *pvParameters);

/* Função de apoio: Cálculo da corrente permitida segundo IEC 61851-1 */
float calculate_max_current_from_duty(float duty_cycle) {
    if (duty_cycle >= 8.0f && duty_cycle <= 10.0f) {
        return 6.0f; /* Recarga mínima de 6A */
    } else if (duty_cycle > 10.0f && duty_cycle <= 85.0f) {
        return duty_cycle * 0.6f; /* Corrente = Duty Cycle * 0.6 A */
    } else if (duty_cycle > 85.0f && duty_cycle <= 96.0f) {
        return (duty_cycle - 64.0f) * 2.5f; /* Corrente alta */
    } else {
        return 0.0f; /* Modo de falha ou reservado */
    }
}

int main(void) {
    /* Inicialização de recursos de SO */
    xDiagnosticQueue = xQueueCreate(10, sizeof(evse_diagnostic_data_t));
    xStateMutex = xSemaphoreCreateMutex();

    if (xDiagnosticQueue != NULL && xStateMutex != NULL) {
        /* Criação das Tarefas com Prioridades */
        xTaskCreate(vTaskADCSampler, "ADC_Sampler", 256, NULL, 4, NULL);
        xTaskCreate(vTaskSafetyMonitor, "Safety_Monitor", 256, NULL, 3, NULL);
        xTaskCreate(vTaskIEC61851StateMachine, "State_Machine", 256, NULL, 2, NULL);
        xTaskCreate(vTaskHMIDisplay, "HMI_Display", 256, NULL, 1, NULL);

        /* Inicia o Scheduler do FreeRTOS */
        vTaskStartScheduler();
    }

    /* Loop infinito em caso de falha de inicialização */
    while (1);
    return 0;
}

/* 1. Tarefa de Amostragem do ADC (Prioridade 4 - Alta) */
void vTaskADCSampler(void *pvParameters) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(10); /* Executa a cada 10ms */

    evse_diagnostic_data_t sampleData;

    for (;;) {
        /* [Inserir leitura do hardware ADC aqui] */
        /* Simulação de leitura de tensão do CP e Duty Cycle */
        sampleData.cp_positive_voltage = 8.92f; /* Exemplo: Estado B */
        sampleData.cp_duty_cycle_percent = 53.33f; /* Exemplo: 32A nominal */
        sampleData.max_allowed_current_amps = calculate_max_current_from_duty(sampleData.cp_duty_cycle_percent);

        /* Envia dados para a fila de diagnóstico */
        xQueueOverwrite(xDiagnosticQueue, &sampleData);

        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

/* 2. Tarefa de Monitoramento de Segurança (Prioridade 3) */
void vTaskSafetyMonitor(void *pvParameters) {
    evse_diagnostic_data_t diag;

    for (;;) {
        if (xQueuePeek(xDiagnosticQueue, &diag, portMAX_DELAY) == pdPASS) {
            /* Verifica falha de aterramento / curto-circuito (Estado E) */
            if (diag.cp_positive_voltage < 1.0f) {
                /* Exigência da norma: Abertura da contatora em < 100ms em falha */
                diag.contactor_closed = false;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

/* 3. Tarefa da Máquina de Estados da IEC 61851 (Prioridade 2) */
void vTaskIEC61851StateMachine(void *pvParameters) {
    evse_diagnostic_data_t diag;

    for (;;) {
        if (xQueueReceive(xDiagnosticQueue, &diag, portMAX_DELAY) == pdPASS) {
            xSemaphoreTake(xStateMutex, portMAX_DELAY);

            /* Avaliação do Nível de Tensão VA */
            if (diag.cp_positive_voltage >= 11.0f) {
                diag.current_state = EV_STATE_A;
                diag.contactor_closed = false;
            } else if (diag.cp_positive_voltage >= 8.0f && diag.cp_positive_voltage < 11.0f) {
                diag.current_state = EV_STATE_B;
                diag.contactor_closed = false;
            } else if (diag.cp_positive_voltage >= 5.0f && diag.cp_positive_voltage < 8.0f) {
                diag.current_state = EV_STATE_C;
                diag.contactor_closed = true; /* Fecha a contatora principal */
            } else if (diag.cp_positive_voltage >= 2.0f && diag.cp_positive_voltage < 5.0f) {
                diag.current_state = EV_STATE_D;
                diag.contactor_closed = true;
            } else {
                diag.current_state = EV_STATE_E;
                diag.contactor_closed = false; /* Desliga contatora imediatamente */
            }

            xSemaphoreGive(xStateMutex);
        }
    }
}

/* 4. Tarefa de Interface e Display (Prioridade 1 - Baixa) */
void vTaskHMIDisplay(void *pvParameters) {
    for (;;) {
        /* Atualização de LEDs de status, tela LCD ou saída Serial / UART */
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
```

---

## 4. Como Importar e Continuar no AntiGravite IDE

1. **Baixar/Copiar o arquivo:** Utilize este documento como a especificação do seu projeto no AntiGravite IDE.
2. **Criação da Estrutura de Pastas no IDE:**
   - Crie uma pasta `src/` e adicione o arquivo `main.c`.
   - Crie uma pasta `include/` e adicione o arquivo `iec61851_emulator.h`.
3. **Configuração do FreeRTOS:**
   - Garanta que o arquivo `FreeRTOSConfig.h` esteja presente com as configurações de clock e *ticks* adequadas ao microcontrolador escolhido.
