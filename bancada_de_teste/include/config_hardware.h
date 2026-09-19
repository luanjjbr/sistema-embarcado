#ifndef CONFIG_HARDWARE_H
#define CONFIG_HARDWARE_H

#include <Arduino.h>

#if defined(ESP32) || defined(PLACA_ESP32)
    // =========================================================================
    // CONFIGURAÇÕES ESPECÍFICAS: ESP32 (Xtensa Dual-Core 32 bits, 3.3V)
    // =========================================================================
    #define HARDWARE_IDENTIFICADOR       "ESP32-DevKit"
    #define HARDWARE_ARQUITETURA         "Xtensa 32-bit (Dual Core)"
    #define HARDWARE_TENSAO_OPERACAO_MV  3300
    #define HARDWARE_ADC_BITS            12
    #define HARDWARE_ADC_MAX_VALOR       4095
    #define HARDWARE_TOTAL_CANAIS_ADC    4

    // Mapeamento de pinos
    #ifndef LED_BUILTIN
        #define PINO_LED_STATUS          2   // GPIO 2 é o LED padrão da maioria das DevKits ESP32
    #else
        #define PINO_LED_STATUS          LED_BUILTIN
    #endif

    #define PINO_BOTAO_TESTE             0   // Botão BOOT do ESP32 (GPIO 0 com pull-up interno)

    // Canais ADC de teste selecionados (apenas ADC1 para compatibilidade com Wi-Fi)
    const uint8_t PINOS_ADC_TESTE[HARDWARE_TOTAL_CANAIS_ADC] = {34, 35, 32, 33};

    // Função de memória livre para ESP32
    inline uint32_t obterMemoriaLivreBytes() {
        return ESP.getFreeHeap();
    }

    // Frequência de clock em MHz
    inline uint32_t obterClockCpuMHz() {
        return getCpuFrequencyMhz();
    }

    // Reinicialização por software
    inline void reiniciarSistema() {
        ESP.restart();
    }

#elif defined(__AVR_ATmega2560__) || defined(PLACA_MEGA)
    // =========================================================================
    // CONFIGURAÇÕES ESPECÍFICAS: ARDUINO MEGA 2560 (AVR 8 bits, 5V)
    // =========================================================================
    #include <avr/wdt.h>

    #define HARDWARE_IDENTIFICADOR       "Arduino Mega 2560"
    #define HARDWARE_ARQUITETURA         "AVR 8-bit (ATmega2560)"
    #define HARDWARE_TENSAO_OPERACAO_MV  5000
    #define HARDWARE_ADC_BITS            10
    #define HARDWARE_ADC_MAX_VALOR       1023
    #define HARDWARE_TOTAL_CANAIS_ADC    8   // Exibe 8 canais principais (o Mega possui até 16)

    // Mapeamento de pinos
    #define PINO_LED_STATUS              LED_BUILTIN // Pino 13
    #define PINO_BOTAO_TESTE             2           // Pino digital 2 com interrupção externa

    const uint8_t PINOS_ADC_TESTE[HARDWARE_TOTAL_CANAIS_ADC] = {A0, A1, A2, A3, A4, A5, A6, A7};

    // Cálculo de SRAM livre para microcontroladores AVR
    inline uint32_t obterMemoriaLivreBytes() {
        extern int __heap_start, *__brkval;
        int v;
        return (uint32_t)( (int)&v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval) );
    }

    inline uint32_t obterClockCpuMHz() {
        return (uint32_t)(F_CPU / 1000000UL);
    }

    inline void reiniciarSistema() {
        wdt_enable(WDTO_15MS);
        while (1) {} // Aguarda o Watchdog disparar o reset
    }

#elif defined(__AVR_ATmega328P__) || defined(PLACA_NANO)
    // =========================================================================
    // CONFIGURAÇÕES ESPECÍFICAS: ARDUINO NANO (AVR 8 bits, 5V)
    // =========================================================================
    #include <avr/wdt.h>

    #define HARDWARE_IDENTIFICADOR       "Arduino Nano (ATmega328P)"
    #define HARDWARE_ARQUITETURA         "AVR 8-bit (ATmega328P)"
    #define HARDWARE_TENSAO_OPERACAO_MV  5000
    #define HARDWARE_ADC_BITS            10
    #define HARDWARE_ADC_MAX_VALOR       1023
    #define HARDWARE_TOTAL_CANAIS_ADC    6   // A0 a A5 (ou até A7 se disponível no Nano)

    // Mapeamento de pinos
    #define PINO_LED_STATUS              LED_BUILTIN // Pino 13
    #define PINO_BOTAO_TESTE             2           // Pino digital 2 (D2)

    const uint8_t PINOS_ADC_TESTE[HARDWARE_TOTAL_CANAIS_ADC] = {A0, A1, A2, A3, A4, A5};

    // Cálculo de SRAM livre para microcontroladores AVR
    inline uint32_t obterMemoriaLivreBytes() {
        extern int __heap_start, *__brkval;
        int v;
        return (uint32_t)( (int)&v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval) );
    }

    inline uint32_t obterClockCpuMHz() {
        return (uint32_t)(F_CPU / 1000000UL);
    }

    inline void reiniciarSistema() {
        wdt_enable(WDTO_15MS);
        while (1) {} // Aguarda o Watchdog disparar o reset
    }

#else
    #error "Placa não suportada ou não identificada. Verifique as flags no platformio.ini."
#endif

#endif // CONFIG_HARDWARE_H
