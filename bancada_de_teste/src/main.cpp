/**
 * ============================================================================
 * Projeto: Blink com FreeRTOS
 * Plataforma: ESP32 (DevKit)
 * Disciplina: Sistemas Embarcados
 * ============================================================================
 */

#include <Arduino.h>

// Definição do pino do LED
#ifndef LED_BUILTIN
    #define PINO_LED 2  // GPIO 2 é o LED padrão da maioria das placas ESP32 DevKit
#else
    #define PINO_LED LED_BUILTIN
#endif

// Handle para gerenciar a tarefa do FreeRTOS
TaskHandle_t xTaskBlinkHandle = NULL;

/**
 * @brief Tarefa do FreeRTOS responsável por piscar o LED
 * @param pvParameters Parâmetros passados para a tarefa (não utilizado)
 */
void vTaskBlink(void *pvParameters) {
    // Configura o pino como saída
    pinMode(PINO_LED, OUTPUT);

    // Loop infinito da tarefa (padrão de tarefas FreeRTOS)
    for (;;) {
        digitalWrite(PINO_LED, HIGH);
        // vTaskDelay bloqueia a tarefa liberando o núcleo para outras tarefas
        vTaskDelay(pdMS_TO_TICKS(500)); // Aguarda 500ms

        digitalWrite(PINO_LED, LOW);
        vTaskDelay(pdMS_TO_TICKS(500)); // Aguarda 500ms
    }
}

void setup() {
    // Inicialização da porta serial para depuração
    Serial.begin(115200);
    delay(500);

    Serial.println();
    Serial.println("==================================================");
    Serial.println("       ESP32 - BLINK UTILIZANDO FREERTOS          ");
    Serial.println("==================================================");
    Serial.printf("Pino do LED: GPIO %d\n", PINO_LED);
    Serial.println("Criando a tarefa vTaskBlink...");

    // Criação da tarefa no FreeRTOS
    BaseType_t resultado = xTaskCreatePinnedToCore(
        vTaskBlink,         // Função que implementa a tarefa
        "Task_Blink",       // Nome descritivo da tarefa (útil para debug)
        2048,               // Tamanho da stack em bytes (ESP32 IDF usa bytes)
        NULL,               // Parâmetro passado à tarefa
        1,                  // Prioridade da tarefa (1 = baixa prioridade)
        &xTaskBlinkHandle,  // Handle de controle da tarefa
        1                   // Núcleo do processador (0 ou 1, 1 é o núcleo padrão da aplicação)
    );

    if (resultado == pdPASS) {
        Serial.println("[OK] Tarefa criada e em execução com sucesso!");
    } else {
        Serial.println("[ERRO] Falha ao criar a tarefa do FreeRTOS!");
    }

    Serial.println("==================================================");
}

void loop() {
    // No ESP32, o setup() e loop() rodam dentro de sua própria tarefa (loopTask).
    // Como o blink roda em sua própria tarefa independente, podemos deixar o loop dormindo
    // ou usá-lo para outras tarefas futuras.
    //vTaskDelay(pdMS_TO_TICKS(1000));
}
