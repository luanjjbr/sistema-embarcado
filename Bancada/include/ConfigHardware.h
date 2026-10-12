/**
 * ============================================================================
 * Projeto: Bancada
 * Arquivo: ConfigHardware.h
 * Finalidade: Mapeamento de pinos e configurações de hardware
 * Microcontroladores suportados: Arduino Nano (ATmega328P) e Arduino Mega 2560
 * ============================================================================
 */

#ifndef CONFIG_HARDWARE_H
#define CONFIG_HARDWARE_H

#include <Arduino.h>

// ============================================================================
// 1. CONFIGURAÇÕES DE COMUNICAÇÃO SERIAL
// ============================================================================
#ifndef SERIAL_BAUD_RATE
#define SERIAL_BAUD_RATE 9600
#endif

// ============================================================================
// 2. SELEÇÃO DE PINAGEM E IDENTIFICAÇÃO DE PLACA
// ============================================================================
#if defined(PLACA_MEGA) || defined(__AVR_ATmega2560__)
    #define NOME_PLACA "Arduino Mega 2560 (5V)"
    #define PINO_LED_STATUS 13
#elif defined(PLACA_NANO) || defined(__AVR_ATmega328P__)
    #define NOME_PLACA "Arduino Nano (5V)"
    #define PINO_LED_STATUS 13
#else
    #define NOME_PLACA "Arduino AVR Generico (5V)"
    #define PINO_LED_STATUS 13
#endif

// Tamanho padrão de stack para tarefas em microcontroladores AVR (em words)
#define STACK_BLINK 128
#define STACK_CABO  128

// ============================================================================
// 3. PINOS DO EMULADOR DE CABO - PINO PP (IEC 62196-2)
// ============================================================================
// Acionamento das chaves MOSFET S1 a S6 do barramento de resistores para PE
#define PINO_CABO_S1    4    // S1: 4700 Ohms (Desconectado nominal)
#define PINO_CABO_S2    5    // S2: 1500 Ohms (Cabo 13 A)
#define PINO_CABO_S3    6    // S3:  680 Ohms (Cabo 20 A)
#define PINO_CABO_S4    7    // S4:  220 Ohms (Cabo 32 A)
#define PINO_CABO_S5    8    // S5:  100 Ohms (Cabo 63 A / 70 A)
#define PINO_CABO_S6    9    // S6:   47 Ohms (Simulação de Falha / Curto)

// ============================================================================
// 4. LÓGICA DE ACIONAMENTO DOS RELÉS (5V)
// ============================================================================
// Nível lógico para o driver de relé (HIGH para NPN S8050 / LOW para optoacoplador invertido)
#define NIVEL_RELE_ATIVO    HIGH
#define NIVEL_RELE_INATIVO  LOW

// ============================================================================
// 5. PINOS DO EMULADOR DE VEÍCULO - PINO CP (IEC 61851-1 ANEXO A)
// ============================================================================
// Acionamento dos 10 relés de comutação de estados e tolerâncias operacionais
#if defined(PLACA_MEGA) || defined(__AVR_ATmega2560__)
    #define PINO_CP_S7      22   // S7: Conexão PE (Aberto = Estado AX, 12V)
    #define PINO_CP_S8      23   // S8: Falha 120R direto ao PE (Fechado = Estado E, 1V)
    #define PINO_CP_S9_1    24   // S9.1: Bypass 1870R -> 2740R (Estado BX Nominal, 9V)
    #define PINO_CP_S9_2    25   // S9.2: Bypass 1870R+920R -> 1820R (Estado BX Limite Inf, 8V)
    #define PINO_CP_S10_C   26   // S10.2: Habilita barramento do Estado C
    #define PINO_CP_S10_D   27   // S10.1: Habilita barramento do Estado D
    #define PINO_CP_S11_1   28   // S11.1: Bypass 422R -> 1301R (Estado CX Nominal, 6V)
    #define PINO_CP_S11_2   29   // S11.2: Bypass 422R+392R -> 909R (Estado CX Limite Inf, 5V)
    #define PINO_CP_S12_1   30   // S12.1: Bypass 178R -> 270R (Estado DX Nominal, 3V)
    #define PINO_CP_S12_2   31   // S12.2: Bypass 178R+130R -> 140R (Estado DX Limite Inf, 2V)
#endif

#endif // CONFIG_HARDWARE_H

