/**
 * ============================================================================
 * Projeto: Bancada de Teste - Estação de Recarga Veicular (IEC 61851-1)
 * Arquivo: main.cpp
 * Módulo: Aplicação Multitarefa com FreeRTOS e POO
 * Microcontroladores: Arduino Nano (ATmega328P) / ESP32 DevKit
 * ============================================================================
 */

#include <Arduino.h>
#include "ConfigHardware.h"
#include "EmuladorCabo.h"

// ============================================================================
// 1. INCLUSÃO DAS BIBLIOTECAS DO FREERTOS POR ARQUITETURA
// ============================================================================
#if defined(ESP32)
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// Tamanho das pilhas (stack) no ESP32 (em bytes - 320 KB RAM disponível)
#define STACK_HEARTBEAT 2048
#define STACK_SERIAL 3072

#else
#include <Arduino_FreeRTOS.h>
#include <task.h>

// Tamanho das pilhas (stack) no Arduino Nano (em bytes/words - 2 KB RAM total)
#define STACK_HEARTBEAT 128
#define STACK_SERIAL 256
#endif

// ============================================================================
// 2. INSTANCIAÇÃO DOS OBJETOS DA BANCADA (POO)
// ============================================================================
EmuladorCabo cabo(PINO_CABO_S1, PINO_CABO_S2, PINO_CABO_S3,
                  PINO_CABO_S4, PINO_CABO_S5, PINO_CABO_S6);

// ============================================================================
// 3. FUNÇÕES DE INTERFACE SERIAL (CLI)
// ============================================================================
void exibirMenu()
{
    Serial.println();
    Serial.println(F("=================================================="));
    Serial.println(F("      COMANDOS DO EMULADOR DE CABO (PINO PP)      "));
    Serial.println(F("=================================================="));
    Serial.println(F(" cabo 0 | cabo off   -> Desliga tudo (Aberto Total)"));
    Serial.println(F(" cabo 1 | cabo desc  -> S1: 4700R (Desconectado)"));
    Serial.println(F(" cabo 2 | cabo 13a   -> S2: 1500R (Cabo 13 A)"));
    Serial.println(F(" cabo 3 | cabo 20a   -> S3:  680R (Cabo 20 A)"));
    Serial.println(F(" cabo 4 | cabo 32a   -> S4:  220R (Cabo 32 A)"));
    Serial.println(F(" cabo 5 | cabo 63a   -> S5:  100R (Cabo 63 A)"));
    Serial.println(F(" cabo 6 | cabo falha -> S6:   47R (Simula Falha)"));
    Serial.println(F(" cabo status         -> Exibe estado atual"));
    Serial.println(F(" help | ?            -> Mostra este menu"));
    Serial.println(F("=================================================="));
    Serial.print(F("bancada> "));
}

void processarComando(String cmd)
{
    cmd.trim();
    if (cmd.length() == 0)
        return;

    String cmdLower = cmd;
    cmdLower.toLowerCase();

    if (cmdLower == "help" || cmdLower == "?")
    {
        exibirMenu();
        return;
    }

    if (cmdLower.startsWith("cabo "))
    {
        String param = cmdLower.substring(5);
        param.trim();

        int canal = -1;
        if (param == "0" || param == "off")
        {
            canal = 0;
        }
        else if (param == "1" || param == "desc")
        {
            canal = 1;
        }
        else if (param == "2" || param == "13a")
        {
            canal = 2;
        }
        else if (param == "3" || param == "20a")
        {
            canal = 3;
        }
        else if (param == "4" || param == "32a")
        {
            canal = 4;
        }
        else if (param == "5" || param == "63a")
        {
            canal = 5;
        }
        else if (param == "6" || param == "falha")
        {
            canal = 6;
        }
        else if (param == "status")
        {
            Serial.print(F("OK: STATUS "));
            Serial.println(cabo.getDescricao());
            Serial.print(F("bancada> "));
            return;
        }

        if (canal >= 0 && canal <= 6)
        {
            cabo.ativarCanal(canal);
            Serial.print(F("OK: CABO "));
            Serial.print(canal);
            Serial.print(F(" ("));
            Serial.print(cabo.getDescricao());
            Serial.println(F(")"));
        }
        else
        {
            Serial.print(F("ERRO: Cabo inexistente '"));
            Serial.print(param);
            Serial.println(F("'. Opcoes validas: 0 a 6."));
        }
        Serial.print(F("bancada> "));
        return;
    }

    // Qualquer outro comando não reconhecido
    Serial.print(F("ERRO: Comando desconhecido '"));
    Serial.print(cmd);
    Serial.println(F("'. Digite 'help' para comandos."));
    Serial.print(F("bancada> "));
}

// ============================================================================
// 4. TAREFAS FREERTOS (TASKS)
// ============================================================================

/**
 * @brief Tarefa 1: Heartbeat (Pisca o LED para monitorar a saúde do sistema)
 * Prioridade: 1 (Baixa)
 */
void tarefaHeartbeat(void *pvParameters)
{
    (void)pvParameters;
    pinMode(PINO_LED_STATUS, OUTPUT);
    bool estadoLed = LOW;

    for (;;)
    {
        estadoLed = !estadoLed;
        digitalWrite(PINO_LED_STATUS, estadoLed);

        // Bloqueia a tarefa por 500 ms liberando a CPU para o escalonador
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

/**
 * @brief Tarefa 2: Leitura e Processamento de Comandos UART
 * Prioridade: 2 (Média/Alta para resposta rápida aos comandos de teste)
 */
void tarefaSerial(void *pvParameters)
{
    (void)pvParameters;
    String comandoSerial = "";
    comandoSerial.reserve(32); // Evita fragmentação de memória

    for (;;) {
        while (Serial.available() > 0)
        {
            char c = (char)Serial.read();

            if (c == '\r')
                continue;

            if (c == '\n')
            {
                processarComando(comandoSerial);
                comandoSerial = "";
            }
            else
            {
                comandoSerial += c;
            }
        }

        // Aguarda 20 ms antes de checar novamente a Serial, cedendo tempo de CPU
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

// ============================================================================
// 5. INICIALIZAÇÃO DO SISTEMA (SETUP)
// ============================================================================
void setup()
{
    Serial.begin(SERIAL_BAUD_RATE);

    // Inicializa a hardware abstraction layer (HAL) do emulador de cabo
    cabo.begin();

    // Mensagem inicial de inicialização
    Serial.println();
    Serial.println(F("=================================================="));
    Serial.println(F("  BANCADA DE TESTE - COM FREERTOS + POO ATIVOS    "));
    Serial.println(F("=================================================="));
    Serial.print(F(" Placa Detectada: "));
    Serial.println(F(NOME_PLACA));
    Serial.println(F(" S.O. Tempo Real: FreeRTOS Scheduler Habilitado   "));
    Serial.println(F(" Estado Inicial:  PADRAO (Todas as chaves ABERTAS)"));
    Serial.println(F(" Digite 'help' para ver a lista de comandos.      "));
    Serial.println(F("=================================================="));
    Serial.print(F("bancada> "));

    // Criação da Tarefa de Heartbeat
    xTaskCreate(
        tarefaHeartbeat,
        "Heartbeat",
        STACK_HEARTBEAT,
        NULL,
        1,
        NULL);

    // Criação da Tarefa de Comunicação Serial
    xTaskCreate(
        tarefaSerial,
        "SerialCLI",
        STACK_SERIAL,
        NULL,
        2,
        NULL);

    // No Arduino Nano (AVR), o escalonador inicia automaticamente após o fim do setup().
    // No ESP32, o escalonador já está rodando antes do setup().
}

// ============================================================================
// 6. LOOP PRINCIPAL
// ============================================================================
void loop() {
// Com o FreeRTOS ativo, todo o processamento reside nas tarefas.
#if defined(ESP32)
    vTaskDelay(pdMS_TO_TICKS(1000));
#endif
}
