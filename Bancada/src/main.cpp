/**
 * ============================================================================
 * Projeto: Bancada
 * Arquivo: main.cpp
 * Finalidade: Interface Serial por Comandos (CLI) com FreeRTOS, EmuladorCabo,
 *             Buffer Estático em C e Proteção Concorrente de UART via Mutex
 * Microcontroladores: Arduino Nano (ATmega328P) / Arduino Mega 2560
 * ============================================================================
 */

#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <string.h>
#include <ctype.h>
#include "ConfigHardware.h"
#include "EmuladorCabo.h"

// ============================================================================
// 1. RECURSOS DE SINCRONIZAÇÃO FREERTOS
// ============================================================================
SemaphoreHandle_t xSerialMutex = NULL;

// ============================================================================
// 2. INSTANCIAÇÃO DOS OBJETOS DA BANCADA (POO)
// ============================================================================
EmuladorCabo cabo(PINO_CABO_S1, PINO_CABO_S2, PINO_CABO_S3,
                  PINO_CABO_S4, PINO_CABO_S5, PINO_CABO_S6);

// ============================================================================
// 3. FUNÇÕES DE PROCESSAMENTO SERIAL (CLI)
// ============================================================================

void exibirHelp()
{
    Serial.println();
    Serial.println(F("=================================================="));
    Serial.println(F("            COMANDOS DA BANCADA (IEC 62196)       "));
    Serial.println(F("=================================================="));
    Serial.println(F(" cabo 0      -> Circuito Aberto (Desliga todas as chaves)"));
    Serial.println(F(" cabo 1      -> S1: Desconectado nominal (4700 Ohms)"));
    Serial.println(F(" cabo 2      -> S2: Cabo 13 A (1500 Ohms)"));
    Serial.println(F(" cabo 3      -> S3: Cabo 20 A (680 Ohms)"));
    Serial.println(F(" cabo 4      -> S4: Cabo 32 A (220 Ohms)"));
    Serial.println(F(" cabo 5      -> S5: Cabo 63 A (100 Ohms)"));
    Serial.println(F(" cabo 6      -> S6: Falha de Isolacao / Curto (47 Ohms)"));
    Serial.println(F(" cabo status -> Exibe relatorio do estado atual"));
    Serial.println(F(" help        -> Exibe este menu explicativo"));
    Serial.println(F("=================================================="));
}

void processarComando(char *comando)
{
    // Remove espaços iniciais
    while (isspace((unsigned char)*comando)) {
        comando++;
    }

    // Remove espaços finais
    int len = strlen(comando);
    while (len > 0 && isspace((unsigned char)comando[len - 1])) {
        comando[--len] = '\0';
    }

    if (len == 0) {
        return;
    }

    // Converte para minúsculas para comparação uniforme
    for (int i = 0; i < len; i++) {
        comando[i] = tolower((unsigned char)comando[i]);
    }

    // 1. Comando de Ajuda
    if (strcmp(comando, "help") == 0 || strcmp(comando, "?") == 0)
    {
        exibirHelp();
        return;
    }

    // 2. Comandos do tipo "cabo ..."
    if (strncmp(comando, "cabo", 4) == 0)
    {
        char *arg = comando + 4;
        while (isspace((unsigned char)*arg)) {
            arg++;
        }

        // Se digitou apenas "cabo" sem argumento
        if (*arg == '\0')
        {
            Serial.println(F("ERRO: Cabo"));
            return;
        }

        // Subcomando de status
        if (strcmp(arg, "status") == 0)
        {
            cabo.imprimirStatus();
            return;
        }

        // Subcomando numérico "cabo 0" a "cabo 6"
        if (arg[0] >= '0' && arg[0] <= '6' && arg[1] == '\0')
        {
            int canal = arg[0] - '0';
            cabo.ativarCanal(canal);
            Serial.println(F("OK"));
            return;
        }
        else
        {
            Serial.println(F("ERRO: Cabo"));
            return;
        }
    }

    // 3. Qualquer outro comando desconhecido
    Serial.println(F("ERRO"));
}

// ============================================================================
// 4. TAREFAS FREERTOS
// ============================================================================

void tarefaBlink(void *pvParameters)
{
    (void)pvParameters;
    pinMode(PINO_LED_STATUS, OUTPUT);
    bool estadoLed = LOW;

    for (;;)
    {
        estadoLed = !estadoLed;
        digitalWrite(PINO_LED_STATUS, estadoLed);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void tarefaSerial(void *pvParameters)
{
    (void)pvParameters;
    cabo.begin();

    // Buffer estático em C (Zero fragmentação de heap)
    char bufferEntrada[32];
    uint8_t indiceBuffer = 0;

    for (;;)
    {
        while (Serial.available() > 0)
        {
            char c = (char)Serial.read();

            if (c == '\r')
            {
                continue; // Ignora retorno de carro
            }

            if (c == '\n')
            {
                bufferEntrada[indiceBuffer] = '\0';

                if (xSerialMutex != NULL && xSemaphoreTake(xSerialMutex, portMAX_DELAY) == pdTRUE)
                {
                    processarComando(bufferEntrada);
                    xSemaphoreGive(xSerialMutex);
                }

                indiceBuffer = 0; // Reinicia o buffer
            }
            else
            {
                // Proteção contra estouro de buffer (Buffer Overflow)
                if (indiceBuffer < (sizeof(bufferEntrada) - 1))
                {
                    bufferEntrada[indiceBuffer++] = c;
                }
            }
        }

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

// ============================================================================
// 5. INICIALIZAÇÃO DO SISTEMA (SETUP)
// ============================================================================
void setup()
{
    Serial.begin(SERIAL_BAUD_RATE);

    xSerialMutex = xSemaphoreCreateMutex();

    Serial.println();
    Serial.println(F("=================================================="));
    Serial.println(F("    PROJETO BANCADA - EMULADOR DE CABO IEC 62196  "));
    Serial.println(F("=================================================="));
    Serial.print(F(" Placa Detectada: "));
    Serial.println(F(NOME_PLACA));
    Serial.print(F(" Baud Rate:       "));
    Serial.println(SERIAL_BAUD_RATE);
    Serial.println(F(" Mutex Serial:    HABILITADO                      "));
    Serial.println(F(" Buffer Estatico: HABILITADO (32 bytes)           "));
    Serial.println(F(" Digite 'help' para ver os comandos disponiveis.  "));
    Serial.println(F("=================================================="));

    if (xSerialMutex != NULL)
    {
        xTaskCreate(
            tarefaBlink,
            "Blink",
            STACK_BLINK,
            NULL,
            1,
            NULL
        );

        xTaskCreate(
            tarefaSerial,
            "SerialCLI",
            STACK_CABO,
            NULL,
            1,
            NULL
        );
    }
    else
    {
        Serial.println(F("FALHA CRITICA: Impossivel alocar Mutex Serial!"));
    }
}

// ============================================================================
// 6. LOOP PRINCIPAL
// ============================================================================
void loop()
{
}
