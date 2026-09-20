/**
 * ============================================================================
 * Projeto: Bancada
 * Arquivo: main.cpp
 * Finalidade: Interface Serial por Comandos (CLI) com FreeRTOS e EmuladorCabo
 * Microcontroladores: Arduino Nano (ATmega328P) / Arduino Mega 2560
 * ============================================================================
 */

#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <task.h>
#include "ConfigHardware.h"
#include "EmuladorCabo.h"

// ============================================================================
// 1. INSTANCIAÇÃO DOS OBJETOS DA BANCADA (POO)
// ============================================================================
EmuladorCabo cabo(PINO_CABO_S1, PINO_CABO_S2, PINO_CABO_S3,
                  PINO_CABO_S4, PINO_CABO_S5, PINO_CABO_S6);

// ============================================================================
// 2. FUNÇÕES DE PROCESSAMENTO SERIAL (CLI)
// ============================================================================

void exibirHelp()
{
    Serial.println();
    Serial.println(F("=================================================="));
    Serial.println(F("            COMANDOS DA BANCADA (IEC 62196)       "));
    Serial.println(F("=================================================="));
    Serial.println(F(" cabo 0  -> Circuito Aberto (Desliga todas as chaves)"));
    Serial.println(F(" cabo 1  -> S1: Desconectado nominal (4700 Ohms)"));
    Serial.println(F(" cabo 2  -> S2: Cabo 13 A (1500 Ohms)"));
    Serial.println(F(" cabo 3  -> S3: Cabo 20 A (680 Ohms)"));
    Serial.println(F(" cabo 4  -> S4: Cabo 32 A (220 Ohms)"));
    Serial.println(F(" cabo 5  -> S5: Cabo 63 A (100 Ohms)"));
    Serial.println(F(" cabo 6  -> S6: Falha de Isolacao / Curto (47 Ohms)"));
    Serial.println(F(" help    -> Exibe este menu explicativo"));
    Serial.println(F("=================================================="));
}

void processarComando(String comando)
{
    comando.trim();
    if (comando.length() == 0)
    {
        return;
    }

    String comandoLower = comando;
    comandoLower.toLowerCase();

    // 1. Comando de Ajuda
    if (comandoLower == "help" || comandoLower == "?")
    {
        exibirHelp();
        return;
    }

    // 2. Comandos do tipo "cabo ..."
    if (comandoLower.startsWith("cabo"))
    {
        // Se digitou apenas "cabo" sem argumento
        if (comandoLower.length() <= 4)
        {
            Serial.println(F("ERRO: Cabo"));
            return;
        }

        // Extrai o que foi digitado após "cabo "
        String arg = comandoLower.substring(4);
        arg.trim();

        // Se o argumento for um único caractere entre '0' e '6'
        if (arg.length() == 1 && arg[0] >= '0' && arg[0] <= '6')
        {
            int canal = arg[0] - '0';
            cabo.ativarCanal(canal);
            Serial.println(F("OK"));
            return;
        }
        else
        {
            // Canal fora de 0 a 6 ou argumento inválido
            Serial.println(F("ERRO: Cabo"));
            return;
        }
    }

    // 3. Qualquer outro comando desconhecido
    Serial.println(F("ERRO"));
}

// ============================================================================
// 3. TAREFAS FREERTOS
// ============================================================================

/**
 * @brief Tarefa 1: Heartbeat (Pisca o LED do pino 13 a cada 500 ms)
 */
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

/**
 * @brief Tarefa 2: Interface Serial (CLI) não-bloqueante
 */
void tarefaSerial(void *pvParameters)
{
    (void)pvParameters;
    cabo.begin();

    String bufferEntrada = "";
    bufferEntrada.reserve(32); // Evita fragmentação de heap no AVR

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
                processarComando(bufferEntrada);
                bufferEntrada = "";
            }
            else
            {
                bufferEntrada += c;
            }
        }

        // Cede 20 ms de CPU para outras tarefas do FreeRTOS
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

// ============================================================================
// 4. INICIALIZAÇÃO DO SISTEMA (SETUP)
// ============================================================================
void setup()
{
    Serial.begin(SERIAL_BAUD_RATE);

    Serial.println();
    Serial.println(F("=================================================="));
    Serial.println(F("    PROJETO BANCADA - EMULADOR DE CABO IEC 62196  "));
    Serial.println(F("=================================================="));
    Serial.print(F(" Placa Detectada: "));
    Serial.println(F(NOME_PLACA));
    Serial.print(F(" Baud Rate:       "));
    Serial.println(SERIAL_BAUD_RATE);
    Serial.println(F(" Digite 'help' para ver os comandos disponiveis.  "));
    Serial.println(F("=================================================="));

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

// ============================================================================
// 5. LOOP PRINCIPAL
// ============================================================================
void loop()
{
    // Escalonador preemptivo gerencia as tarefas
}
