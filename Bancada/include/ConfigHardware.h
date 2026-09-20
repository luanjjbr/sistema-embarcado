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

#endif // CONFIG_HARDWARE_H

