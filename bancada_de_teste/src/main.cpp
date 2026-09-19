/**
 * ============================================================================
 * Projeto: Blink com FreeRTOS Multi-Plataforma
 * Suporte: ESP32 | Arduino Nano | Arduino Mega 2560
 * Disciplina: Sistemas Embarcados
 * ============================================================================
 */

#include <Arduino.h>

// Inclusão condicional do FreeRTOS e ajuste de pilha conforme a arquitetura
#if defined(ESP32) || defined(PLACA_ESP32)
// ESP32: FreeRTOS nativo do ESP-IDF
#define NOME_PLATAFORMA "ESP32 (Xtensa Dual-Core)"
#ifndef LED_BUILTIN
#define PINO_LED 2 // GPIO 2 padrão da maioria das DevKits
#else
#define PINO_LED LED_BUILTIN
#endif
// Pilha no ESP-IDF é medida em BYTES
#define TAMANHO_PILHA_TASK 2048
#else
// Arduino AVR: Biblioteca Arduino_FreeRTOS
#include <Arduino_FreeRTOS.h>
#include <util/delay.h>
#define NOME_PLATAFORMA "Arduino AVR (ATmega)"
#define PINO_LED LED_BUILTIN // Pino 13 no Nano / Uno / Mega
// Pilha no AVR FreeRTOS é medida em WORDS (1 word = 2 bytes)
// Usamos configMINIMAL_STACK_SIZE (192 words = 384 bytes)
#define TAMANHO_PILHA_TASK configMINIMAL_STACK_SIZE
#endif

// Garante compatibilidade do macro pdMS_TO_TICKS
#ifndef pdMS_TO_TICKS
#define pdMS_TO_TICKS(ms) ((TickType_t)((uint32_t)(ms) / portTICK_PERIOD_MS))
#endif

// Handle para a tarefa
TaskHandle_t xTaskBlinkHandle = NULL;

/**
 * @brief Tarefa do FreeRTOS responsável por piscar o LED e reportar na Serial
 * @param pvParameters Parâmetros da tarefa
 */
void vTaskBlink(void *pvParameters) {
    (void)pvParameters;

    // Configura o pino do LED como saída
    pinMode(PINO_LED, OUTPUT);

    uint32_t contadorCiclos = 0;

    // Loop infinito da tarefa FreeRTOS
    for (;;) {
        contadorCiclos++;

        // LED LIGADO
        digitalWrite(PINO_LED, HIGH);
        Serial.print(F("[FreeRTOS] Ciclo #"));
        Serial.print(contadorCiclos);
        Serial.println(F(" -> LED LIGADO"));
        vTaskDelay(pdMS_TO_TICKS(500)); // Aguarda 500ms liberando a CPU

        // LED DESLIGADO
        digitalWrite(PINO_LED, LOW);
        Serial.print(F("[FreeRTOS] Ciclo #"));
        Serial.print(contadorCiclos);
        Serial.println(F(" -> LED DESLIGADO"));
        vTaskDelay(pdMS_TO_TICKS(500)); // Aguarda 500ms liberando a CPU
    }
}

void setup()
{
    Serial.begin(115200);

// No AVR FreeRTOS, a biblioteca redefine delay() para chamar vTaskDelay().
// Como o escalonador ainda NÃO começou durante o setup(), usar delay() aqui
// causa congelamento imediato do microcontrolador!
// Usamos delay simples apenas no ESP32 ou _delay_ms() nativo no AVR:
#if defined(ESP32) || defined(PLACA_ESP32)
    delay(100);
#else
    _delay_ms(100);
#endif

    Serial.println();
    Serial.println(F("=================================================="));
    Serial.println(F("      BANCADA - BLINK FREERTOS MULTI-PLACA       "));
    Serial.println(F("=================================================="));
    Serial.print(F("Plataforma ativa: "));
    Serial.println(F(NOME_PLATAFORMA));
    Serial.print(F("Pino do LED:      "));
    Serial.println(PINO_LED);
    Serial.println(F("Criando tarefa vTaskBlink..."));

    // Criação da tarefa no FreeRTOS
#if defined(ESP32) || defined(PLACA_ESP32)
    // No ESP32, vinculamos ao Core 1
    BaseType_t resultado = xTaskCreatePinnedToCore(
        vTaskBlink,
        "Task_Blink",
        TAMANHO_PILHA_TASK,
        NULL,
        1,
        &xTaskBlinkHandle,
        1);
#else
    // No Arduino AVR (Nano / Mega)
    BaseType_t resultado = xTaskCreate(
        vTaskBlink,
        "Task_Blink",
        TAMANHO_PILHA_TASK,
        NULL,
        1,
        &xTaskBlinkHandle);
#endif

    if (resultado == pdPASS) {
        Serial.println(F("[OK] Tarefa FreeRTOS criada com sucesso!"));
        Serial.println(F("Iniciando escalonador..."));
    } else {
        Serial.println(F("[ERRO] Falha ao alocar tarefa FreeRTOS!"));
    }

    Serial.println(F("=================================================="));
    Serial.flush();
}

void loop() {
#if defined(ESP32) || defined(PLACA_ESP32)
    // No ESP32, o loop roda em uma tarefa separada (loopTask)
    vTaskDelay(pdMS_TO_TICKS(1000));
#else
    // No AVR FreeRTOS, loop() é executado pela Idle Task (vApplicationIdleHook).
    // A Idle Task NUNCA pode ser bloqueada (vTaskDelay causaria crash/assert).
    // Portanto, o loop() DEVE ficar vazio no AVR.
#endif
}
