/**
 * ============================================================================
 * Projeto: Bancada de Testes de Cabos EV (IEC 62196-2)
 * Arquivo: Bancada_Teste.ino
 * Ambiente: Arduino IDE (Nano / Uno / Mega 2560)
 * Sistema Operacional: FreeRTOS (feilipu/Arduino_FreeRTOS)
 * Finalidade: Comutacao deterministica do pino PP com intertravamento
 *             Break-Before-Make, comunicacao serial UART e telemetria de status.
 * ============================================================================
 */

#include <Arduino_FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <string.h>
#include <ctype.h>

// ============================================================================
// 1. MAPEAMENTO DE PINOS E CONSTANTES DE HARDWARE
// ============================================================================
#define SERIAL_BAUD_RATE    9600

// LED indicador de atividade do FreeRTOS (Heartbeat)
#define PINO_LED_STATUS     13

// Barramento de MOSFETs do Pino PP (IEC 62196-2)
#define PINO_CABO_S1        4    // S1: 4700 Ohms (Desconectado nominal)
#define PINO_CABO_S2        5    // S2: 1500 Ohms (Cabo 13 A)
#define PINO_CABO_S3        6    // S3:  680 Ohms (Cabo 20 A)
#define PINO_CABO_S4        7    // S4:  220 Ohms (Cabo 32 A)
#define PINO_CABO_S5        8    // S5:  100 Ohms (Cabo 63 A / 70 A)
#define PINO_CABO_S6        9    // S6:   47 Ohms (Simulacao de Falha / Curto)

// Array de pinos para iteracao uniforme
const uint8_t pinosCabo[6] = {
    PINO_CABO_S1,
    PINO_CABO_S2,
    PINO_CABO_S3,
    PINO_CABO_S4,
    PINO_CABO_S5,
    PINO_CABO_S6
};

// ============================================================================
// 2. ESTRUTURAS DE ESTADO E RECURSOS FREERTOS
// ============================================================================
SemaphoreHandle_t xSerialMutex = NULL;

// Estado global do barramento
uint8_t canalAtivo = 0; // 0 = Aberto, 1 a 6 = S1 a S6

// ============================================================================
// 3. LOGICA DE COMUTACAO ELETRICA (BREAK-BEFORE-MAKE)
// ============================================================================
void desligarTodos()
{
    for (uint8_t i = 0; i < 6; i++)
    {
        digitalWrite(pinosCabo[i], LOW);
    }
    canalAtivo = 0;
}

bool ativarCanal(uint8_t canal)
{
    // 1. BREAK: Desliga todas as chaves fisicamente para evitar curto em paralelo
    desligarTodos();

    // 2. Micro-atraso de acomodacao (tempo de turn-off dos MOSFETs)
    delayMicroseconds(5);

    // 3. Se for canal 0 (Aberto Total), finaliza aqui
    if (canal == 0)
    {
        canalAtivo = 0;
        return true;
    }

    // 4. MAKE: Liga exclusivamente o canal solicitado (1 a 6)
    if (canal >= 1 && canal <= 6)
    {
        digitalWrite(pinosCabo[canal - 1], HIGH);
        canalAtivo = canal;
        return true;
    }

    return false;
}

// ============================================================================
// 4. TELEMETRIA E RELATORIO DE STATUS
// ============================================================================
uint16_t getResistenciaNominal(uint8_t canal)
{
    switch (canal)
    {
        case 1: return 4700;
        case 2: return 1500;
        case 3: return 680;
        case 4: return 220;
        case 5: return 100;
        case 6: return 47;
        default: return 0;
    }
}

uint8_t getCorrenteMaxima(uint8_t canal)
{
    switch (canal)
    {
        case 2: return 13;
        case 3: return 20;
        case 4: return 32;
        case 5: return 63;
        default: return 0;
    }
}

void exibirStatus()
{
    Serial.println();
    Serial.println(F("--- [STATUS DA BANCADA] ---"));
    Serial.print(F("Canal Ativo: "));
    Serial.println(canalAtivo);

    Serial.print(F("Resistencia: "));
    Serial.print(getResistenciaNominal(canalAtivo));
    Serial.println(F(" Ohms"));

    Serial.print(F("Capacidade:  "));
    Serial.print(getCorrenteMaxima(canalAtivo));
    Serial.println(F(" A"));

    Serial.print(F("Descricao:   "));
    switch (canalAtivo)
    {
        case 0: Serial.println(F("Circuito Aberto (Sem Cabo)")); break;
        case 1: Serial.println(F("S1: 4700R (Desconectado nominal)")); break;
        case 2: Serial.println(F("S2: 1500R (Cabo 13 A)")); break;
        case 3: Serial.println(F("S3: 680R (Cabo 20 A)")); break;
        case 4: Serial.println(F("S4: 220R (Cabo 32 A)")); break;
        case 5: Serial.println(F("S5: 100R (Cabo 63 A / 70 A)")); break;
        case 6: Serial.println(F("S6: 47R (Simulacao Falha/Curto)")); break;
        default: Serial.println(F("Desconhecido")); break;
    }
    Serial.println(F("---------------------------"));
}

void exibirHelp()
{
    Serial.println();
    Serial.println(F("=================================================="));
    Serial.println(F("         COMANDOS DA BANCADA (ARDUINO IDE)        "));
    Serial.println(F("=================================================="));
    Serial.println(F(" cabo 0      -> Circuito Aberto (Desliga tudo)    "));
    Serial.println(F(" cabo 1      -> S1: Desconectado nominal (4700R)  "));
    Serial.println(F(" cabo 2      -> S2: Cabo 13 A (1500R)             "));
    Serial.println(F(" cabo 3      -> S3: Cabo 20 A (680R)              "));
    Serial.println(F(" cabo 4      -> S4: Cabo 32 A (220R)              "));
    Serial.println(F(" cabo 5      -> S5: Cabo 63 A (100R)              "));
    Serial.println(F(" cabo 6      -> S6: Falha / Curto (47R)           "));
    Serial.println(F(" status      -> Exibe telemetria do estado atual  "));
    Serial.println(F(" cabo status -> Exibe telemetria do estado atual  "));
    Serial.println(F(" help ou ?   -> Exibe este menu explicativo       "));
    Serial.println(F("=================================================="));
}

// ============================================================================
// 5. PARSER SERIAL DETERMINISTICO (CLI)
// ============================================================================
void processarComando(char *cmd)
{
    while (isspace((unsigned char)*cmd)) cmd++;
    int len = strlen(cmd);
    while (len > 0 && isspace((unsigned char)cmd[len - 1])) {
        cmd[--len] = '\0';
    }

    if (len == 0) return;

    for (int i = 0; i < len; i++) {
        cmd[i] = tolower((unsigned char)cmd[i]);
    }

    if (strcmp(cmd, "help") == 0 || strcmp(cmd, "?") == 0)
    {
        exibirHelp();
        return;
    }

    if (strcmp(cmd, "status") == 0)
    {
        exibirStatus();
        return;
    }

    if (strncmp(cmd, "cabo", 4) == 0)
    {
        char *arg = cmd + 4;
        while (isspace((unsigned char)*arg)) arg++;

        if (*arg == '\0')
        {
            Serial.println(F("ERRO: Cabo"));
            return;
        }

        if (strcmp(arg, "status") == 0)
        {
            exibirStatus();
            return;
        }

        if (arg[0] >= '0' && arg[0] <= '6' && arg[1] == '\0')
        {
            uint8_t canal = arg[0] - '0';
            ativarCanal(canal);
            Serial.println(F("OK"));
            return;
        }
        else
        {
            Serial.println(F("ERRO: Cabo"));
            return;
        }
    }

    Serial.println(F("ERRO"));
}

// ============================================================================
// 6. TAREFAS FREERTOS
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
        vTaskDelay(pdMS_TO_TICKS(500)); // Heartbeat a cada 500 ms
    }
}

void tarefaSerial(void *pvParameters)
{
    (void)pvParameters;

    // Inicializacao dos pinos do barramento
    for (uint8_t i = 0; i < 6; i++)
    {
        pinMode(pinosCabo[i], OUTPUT);
        digitalWrite(pinosCabo[i], LOW);
    }
    canalAtivo = 0;

    // Buffer estatico C na pilha da tarefa (Zero Heap / Sem Malloc)
    char bufferEntrada[32];
    uint8_t indiceBuffer = 0;

    for (;;)
    {
        while (Serial.available() > 0)
        {
            char c = (char)Serial.read();

            if (c == '\r') continue;

            if (c == '\n')
            {
                bufferEntrada[indiceBuffer] = '\0';

                if (xSerialMutex != NULL && xSemaphoreTake(xSerialMutex, portMAX_DELAY) == pdTRUE)
                {
                    processarComando(bufferEntrada);
                    xSemaphoreGive(xSerialMutex);
                }

                indiceBuffer = 0;
            }
            else
            {
                if (indiceBuffer < (sizeof(bufferEntrada) - 1))
                {
                    bufferEntrada[indiceBuffer++] = c;
                }
            }
        }

        vTaskDelay(pdMS_TO_TICKS(20)); // Polling suave e nao-bloqueante
    }
}

// ============================================================================
// 7. SETUP E LOOP DO ARDUINO
// ============================================================================
void setup()
{
    Serial.begin(SERIAL_BAUD_RATE);

    xSerialMutex = xSemaphoreCreateMutex();

    Serial.println();
    Serial.println(F("=================================================="));
    Serial.println(F(" BANCADA DE TESTES EV - ARDUINO IDE / FREERTOS    "));
    Serial.println(F("=================================================="));
    Serial.println(F(" Mutex Serial:    ATIVO                           "));
    Serial.println(F(" Buffer Estatico: ATIVO (32 bytes)                "));
    Serial.println(F(" Digite 'help' para comandos ou 'status'          "));
    Serial.println(F("=================================================="));

    if (xSerialMutex != NULL)
    {
        xTaskCreate(
            tarefaBlink,
            "Blink",
            128,   // Stack em words (256 bytes)
            NULL,
            1,     // Prioridade
            NULL
        );

        xTaskCreate(
            tarefaSerial,
            "SerialCLI",
            128,   // Stack em words (256 bytes)
            NULL,
            1,     // Prioridade
            NULL
        );
    }
    else
    {
        Serial.println(F("ERRO CRITICO: Mutex Serial nao criado!"));
    }
}

void loop()
{
    // O FreeRTOS assume o controle preemptivo; loop() permanece vazio.
}

