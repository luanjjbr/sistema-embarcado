/**
 * ============================================================================
 * Projeto: Bancada de Teste - Estação de Recarga Veicular (IEC 61851-1)
 * Arquivo: ConfigHardware.h
 * Finalidade: Mapeamento centralizado de pinos e configurações de hardware
 * Microcontroladores suportados: Arduino Nano / Mega (5V) e ESP32 DevKit (3.3V)
 * ============================================================================
 */

#ifndef CONFIG_HARDWARE_H
#define CONFIG_HARDWARE_H

#include <Arduino.h>

// ============================================================================
// 1. CONFIGURAÇÕES GERAIS DE COMUNICAÇÃO
// ============================================================================
#define SERIAL_BAUD_RATE 115200

// ============================================================================
// 2. SELEÇÃO DE PINAGEM POR MICROCONTROLADOR
// ============================================================================
#if defined(ESP32)
    #define NOME_PLACA "ESP32 DevKit (3.3V)"

    // LED de status (Onboard)
    #define PINO_LED_STATUS 2

    // Pinos de controle das chaves MOSFET do Emulador de Cabo (Pino PP)
    // Conectados aos gates dos MOSFETs com resistores pull-down
    #define PINO_CABO_S1    16   // S1: 4700 Ohms (Desconectado nominal)
    #define PINO_CABO_S2    17   // S2: 1500 Ohms (Cabo 13 A)
    #define PINO_CABO_S3    18   // S3:  680 Ohms (Cabo 20 A)
    #define PINO_CABO_S4    19   // S4:  220 Ohms (Cabo 32 A)
    #define PINO_CABO_S5    21   // S5:  100 Ohms (Cabo 63 A / 70 A)
    #define PINO_CABO_S6    22   // S6:   47 Ohms (Simulação de Falha)

    // Reservados para próximas fases (Emulador do Veículo - Pino CP)
    // #define PINO_CP_ESTADO_A  25
    // #define PINO_CP_ESTADO_B  26
    // #define PINO_CP_ESTADO_C  27
    // #define PINO_CP_ESTADO_D  14
    // #define PINO_CP_PWM_INPUT 34   // Entrada ADC ou leitor de pulso PWM

#else
    #define NOME_PLACA "Arduino Nano / Uno / Mega (5V)"

    // LED de status (Onboard)
    #define PINO_LED_STATUS 13

    // Pinos de controle das chaves MOSFET do Emulador de Cabo (Pino PP)
    // Conectados aos gates dos MOSFETs com resistores pull-down
    #define PINO_CABO_S1    4    // S1: 4700 Ohms (Desconectado nominal)
    #define PINO_CABO_S2    5    // S2: 1500 Ohms (Cabo 13 A)
    #define PINO_CABO_S3    6    // S3:  680 Ohms (Cabo 20 A)
    #define PINO_CABO_S4    7    // S4:  220 Ohms (Cabo 32 A)
    #define PINO_CABO_S5    8    // S5:  100 Ohms (Cabo 63 A / 70 A)
    #define PINO_CABO_S6    9    // S6:   47 Ohms (Simulação de Falha)

    // Reservados para próximas fases (Emulador do Veículo - Pino CP)
    // #define PINO_CP_ESTADO_A  A0
    // #define PINO_CP_ESTADO_B  A1
    // #define PINO_CP_ESTADO_C  A2
    // #define PINO_CP_ESTADO_D  A3
    // #define PINO_CP_PWM_INPUT 2    // Interrupção externa INT0
#endif

#endif // CONFIG_HARDWARE_H

