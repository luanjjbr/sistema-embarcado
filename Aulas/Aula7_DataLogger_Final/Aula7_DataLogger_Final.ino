/*================================================================================
          AULA 7 — DATA LOGGER FINAL COM MALLOC, VDD BANDGAP E CRC-16
        (Baseado e corrigido a partir de sketch_aug24a_malloc_adc.ino)
================================================================================

1. PROBLEMAS E ERROS CRÍTICOS IDENTIFICADOS NO RASCUNHO ORIGINAL:
--------------------------------------------------------------------------------
1.1. Validação de Ponteiro do malloc() Incorreta:
     * No original: `if (i16_adcData == -1)`
     * Motivo: Em C/C++, `malloc()` retorna um ponteiro nulo (`NULL` / 0) em caso 
       de falta de memória na Heap, e NUNCA -1. A checagem com -1 era inválida e 
       permitia que ponteiros nulos fossem acessados, causando travamentos fatais.
     * Correção: Verificação estrita com `if (i16_adcData == NULL)`.

1.2. Buffer de Amostras Estático (Sem Avanço Temporal):
     * No original: A aquisição escrevia sempre nas primeiras posições de 
       `i16_adcData[u16_idx]`, sobrescrevendo o mesmo registro e ignorando a área 
       alocada para o histórico de amostras (`i16_adcAQSLen`).
     * Correção: Implementado endereçamento bidimensional mapeado em buffer contínuo:
       `offset = (indice_amostra * elementos_por_amostra) + canal`.

1.3. Ausência da Transmissão Periódica (ul_timerAQSEND):
     * No original: As variáveis de temporização de transmissão (`ul_timerAQSEND` e 
       `ul_timerAQSENDFIX = 5000`) foram declaradas mas nunca utilizadas no `loop()`.
     * Correção: Implementado timer não-bloqueante via `millis()` que envia o lote 
       completo de amostras acumuladas a cada 5 segundos.

1.4. Ausência de Implementação e Cálculo do CRC-16:
     * No original: O tamanho da amostra reservava `(i16_adcDataLen + 1)` posições 
       (sendo a última para o CRC-16), mas a função de cálculo nunca era chamada.
     * Correção: Implementada função `crc16_modbus()`, calculando e salvando a soma 
       de verificação de integridade no final de cada registro de amostra.

1.5. Configuração Incorreta de Registradores do ATmega328P (ADMUX):
     * No original: `ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);`
     * Motivo: O ATmega328P NÃO possui o bit `MUX4` no registrador `ADMUX`. A seleção 
       da referência interna Bandgap de 1.1V é feita exclusivamente pelos bits 
       `MUX[3:0] = 1110` (MUX3, MUX2, MUX1).
     * Correção: Removido `MUX4` e mantido `_BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1)`.

1.6. Ordem de Leitura dos Registradores do ADC (Atomicidade AVR):
     * No original: `return ((ADCH << 8) | ADCL);`
     * Motivo: O microcontrolador AVR exige que o registrador `ADCL` seja lido ANTES de 
       `ADCH` para travar atomicamente o valor da conversão de 10 bits. Em C/C++, a ordem 
       de avaliação em expressões com deslocamento de bits é indefinida pelo padrão.
     * Correção: Utilizado o registrador unificado `ADCW` (nativo do avr-libc), que 
       garante a leitura atômica e correta dos registradores por hardware.

1.7. Risco de Divisão por Zero e Constrain Negativo em i16_calcVDD():
     * No original: `i32_calcTemp = i32_calcTemp / ((int32_t)(i16_ADC_1v1BG));`
     * Motivo: Caso o ADC retornasse 0 na inicialização, causava divisão por zero.
     * Correção: Proteção com checagem `if (i16_ADC_1v1BG > 0)` e faixa de segurança 
       coerente (2700 mV a 5500 mV).

1.8. Canais Analógicos Reais do Hardware:
     * No original: Tentava ler até 10 canais (`PIN_A0 + u16_idx`), mas o Arduino Uno 
       (DIP) possui apenas 6 canais analógicos (`A0` a `A5`).
     * Correção: Limitado aos 6 canais reais nativos da placa para evitar leituras de 
       pinos digitais inexistentes.

================================================================================*/

#include <Arduino.h>
#include <stdlib.h> // malloc(), free()
#include <string.h> // memset()

// --- Configurações de Hardware e Amostragem Dinâmicas ---
const uint8_t  MAX_ADC_CHANNELS        = 6;    // Limite físico de canais analógicos no Arduino Uno (A0 a A5)
uint8_t        u8_numCanais            = 6;    // Quantidade de canais ativos (configurável via UART: C<n>)
#define        CRC_SLOT_SIZE           1       // 1 slot de int16_t (2 bytes) para o CRC-16

// Temporizações configuráveis (em milissegundos)
unsigned long  ul_tempoAquisicao_ms    = 250;  // Aquisição a cada 250 ms (configurável via 'TA<ms>')
unsigned long  ul_tempoEnvio_ms        = 5000; // Envio UART a cada 5000 ms (configurável via 'TU<ms>')
static const unsigned long TEMPO_VDD_MS = 1000;// Calibração do VDD a cada 1 s

// --- Variáveis Globais de Controle ---
int16_t  *i16_adcData = NULL;          // Ponteiro para o buffer dinâmico contínuo
uint16_t  u16_elementosPorAmostra = 0; // Canais + Slot de CRC-16
uint16_t  u16_totalAmostras       = 0; // Quantidade de amostras no lote (ex: 5000 / 250 = 20)
uint16_t  u16_indiceAmostra       = 0; // Índice da amostra atual sendo adquirida

int16_t   i16_VDDGlobal           = 5000; // Tensão de alimentação calculada em mV

// Temporizadores não-bloqueantes
unsigned long ul_timerUpdateVDD   = 0;
unsigned long ul_timerADCAQ       = 0;
unsigned long ul_timerAQSEND      = 0;

// --- Prototipação de Funções ---
int16_t  i16_readBandGap();
int16_t  i16_calcVDD();
int16_t  i16_convertADC(int16_t i16_raw);
uint16_t crc16_modbus(const uint8_t *pData, size_t length);
void     liberarBuffer();
void     alocarBufferContinuo();
void     transmitirLoteUART();
void     processarComandosUART();

// ============================================================
// Algoritmo CRC-16 (Modbus - Polinômio 0xA001)
// ============================================================
uint16_t crc16_modbus(const uint8_t *pData, size_t length)
{
    uint16_t u16_crc = 0xFFFF;
    for (size_t i = 0; i < length; i++)
    {
        u16_crc ^= pData[i];
        for (uint8_t j = 0; j < 8; j++)
        {
            if (u16_crc & 0x0001)
            {
                u16_crc = (u16_crc >> 1) ^ 0xA001;
            }
            else
            {
                u16_crc >>= 1;
            }
        }
    }
    return u16_crc;
}

// ============================================================
// Gestão de Memória: Alocação Contínua sem Fragmentação
// ============================================================
void liberarBuffer()
{
    if (i16_adcData != NULL)
    {
        free(i16_adcData);
        i16_adcData = NULL;
    }
}

void alocarBufferContinuo()
{
    liberarBuffer();

    u16_elementosPorAmostra = u8_numCanais + CRC_SLOT_SIZE;
    
    // Calcula o número de amostras por lote
    if (ul_tempoAquisicao_ms > 0 && ul_tempoEnvio_ms >= ul_tempoAquisicao_ms)
    {
        u16_totalAmostras = (uint16_t)(ul_tempoEnvio_ms / ul_tempoAquisicao_ms);
    }
    else
    {
        u16_totalAmostras = 1;
    }

    size_t tamanhoTotalBytes = sizeof(int16_t) * u16_elementosPorAmostra * u16_totalAmostras;

    // Alocação de buffer plano (flat buffer) contínuo
    i16_adcData = (int16_t *) malloc(tamanhoTotalBytes);

    // Validação correta: checar contra NULL
    if (i16_adcData == NULL)
    {
        Serial.println(F("[ERRO] Memoria insuficiente no malloc()!"));
        return;
    }

    // Limpa toda a área alocada (reseta para 0)
    memset(i16_adcData, 0, tamanhoTotalBytes);
    u16_indiceAmostra = 0;

    Serial.print(F("[INFO] Buffer de memoria alocado: "));
    Serial.print(tamanhoTotalBytes);
    Serial.print(F(" bytes ("));
    Serial.print(u16_totalAmostras);
    Serial.print(F(" amostras x "));
    Serial.print(u16_elementosPorAmostra);
    Serial.print(F(" words). Canais: "));
    Serial.print(u8_numCanais);
    Serial.print(F(" | Tempo UART: "));
    Serial.print(ul_tempoEnvio_ms);
    Serial.println(F(" ms."));
}

// ============================================================
// Leitura de Tensão de Referência Interna (Bandgap 1.1V do AVR)
// ============================================================
int16_t i16_readBandGap()
{
    // Seleciona VCC como referência e canal Bandgap de 1.1V (MUX3 | MUX2 | MUX1)
    // ATmega328P NÃO usa MUX4.
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
    
    delay(2); // Tempo de estabilização da tensão de referência
    
    ADCSRA |= _BV(ADSC); // Inicia conversão
    while (bit_is_set(ADCSRA, ADSC)); // Aguarda conversão
    
    // ADCW realiza a leitura segura e atômica de ADCL seguido de ADCH
    return ADCW;
}

// ============================================================
// Cálculo Dinâmico do VDD Real (Alimentação do MCU) em mV
// ============================================================
int16_t i16_calcVDD()
{
    int32_t i32_calcConst = 1023L * 1100L; // 1023 passos x 1100 mV (1.1V Bandgap)
    int16_t i16_bgRaw = i16_readBandGap();

    // Proteção contra divisão por zero e leituras anômalas
    if (i16_bgRaw <= 0)
    {
        return 5000; // Valor padrão de fallback (5.0V)
    }

    int32_t i32_vddCalculado = i32_calcConst / (int32_t)i16_bgRaw;
    
    // Limite de segurança físico para microcontrolador ATmega328P
    i32_vddCalculado = constrain(i32_vddCalculado, 2700L, 5500L);
    
    return (int16_t)i32_vddCalculado;
}

// ============================================================
// Conversão de Leitura Bruta (0-1023) para Milivolts (mV)
// ============================================================
int16_t i16_convertADC(int16_t i16_raw)
{
    int32_t i32_mV = ((int32_t)i16_raw * (int32_t)i16_VDDGlobal) / 1023L;
    return (int16_t)i32_mV;
}

// ============================================================
// Transmissão Estruturada do Lote de Amostras via UART (Tudo em mV)
// ============================================================
void transmitirLoteUART()
{
    Serial.println(F("\n================ RELATORIO DE TELEMETRIA (TODAS AS ENTRADAS EM mV) ================"));
    Serial.print(F("Tensão VDD do Sistema: "));
    Serial.print(i16_VDDGlobal);
    Serial.print(F(" mV | Canais Ativos: "));
    Serial.print(u8_numCanais);
    Serial.print(F(" | Amostras: "));
    Serial.print(u16_totalAmostras);
    Serial.print(F(" | Intervalo UART: "));
    Serial.print(ul_tempoEnvio_ms);
    Serial.println(F(" ms"));

    Serial.print(F("Indice | "));
    for (uint8_t ch = 0; ch < u8_numCanais; ch++)
    {
        Serial.print(F("   A"));
        Serial.print(ch);
        Serial.print(F("    | "));
    }
    Serial.println(F(" CRC-16  | Status"));
    Serial.println(F("-----------------------------------------------------------------------------------"));

    for (uint16_t i = 0; i < u16_totalAmostras; i++)
    {
        uint16_t u16_offsetAmostra = i * u16_elementosPorAmostra;

        Serial.print(F("["));
        if (i < 10) Serial.print(F("0"));
        Serial.print(i);
        Serial.print(F("]   | "));

        // Imprime todas as entradas convertidas estritamente em milivolts (mV)
        for (uint8_t ch = 0; ch < u8_numCanais; ch++)
        {
            int16_t canal_mV = i16_adcData[u16_offsetAmostra + ch];
            if (canal_mV < 1000) Serial.print(F(" "));
            if (canal_mV < 100)  Serial.print(F(" "));
            if (canal_mV < 10)   Serial.print(F(" "));
            Serial.print(canal_mV);
            Serial.print(F(" mV | "));
        }

        // Lê e imprime o CRC-16 armazenado no último slot da amostra
        uint16_t u16_crcArmazenado = (uint16_t) i16_adcData[u16_offsetAmostra + u8_numCanais];
        
        // Recalcula o CRC dos dados para validação em tempo real
        uint16_t u16_crcVerificacao = crc16_modbus(
            (const uint8_t *)&i16_adcData[u16_offsetAmostra],
            sizeof(int16_t) * u8_numCanais
        );

        Serial.print(F("0x"));
        if (u16_crcArmazenado < 0x1000) Serial.print(F("0"));
        Serial.print(u16_crcArmazenado, HEX);

        if (u16_crcArmazenado == u16_crcVerificacao)
        {
            Serial.println(F(" | [OK]"));
        }
        else
        {
            Serial.println(F(" | [ERRO CRC]"));
        }
    }
    Serial.println(F("===================================================================================\n"));
}

// ============================================================
// Parser de Comandos UART Interativos
// ============================================================
void processarComandosUART()
{
    if (Serial.available() > 0)
    {
        String cmd = Serial.readStringUntil('\n');
        cmd.trim();

        if (cmd.equalsIgnoreCase("help"))
        {
            Serial.println(F("\n--- Comandos Disponiveis ---"));
            Serial.println(F("C<n>       - Altera o numero de entradas ativas (ex: C4 para 4 entradas: A0 a A3)"));
            Serial.println(F("TU<ms>     - Altera o tempo de envio UART em ms (ex: TU3000 para envio a cada 3s)"));
            Serial.println(F("TA<ms>     - Altera o tempo de amostragem em ms (ex: TA500 para amostrar a cada 500ms)"));
            Serial.println(F("<c>,<tu>     - Altera canais e tempo UART (ex: 4,3000 para 4 entradas a 3s)"));
            Serial.println(F("<c>,<ta>,<tu>- Altera canais, tempo aquisicao e envio (ex: 4,250,5000)"));
            Serial.println(F("read         - Exibe a leitura instantanea de todas as entradas em mV"));
            Serial.println(F("send         - Dispara imediatamente o envio do lote de amostras em mV"));
            Serial.println(F("vdd          - Exibe a calibracao atual da fonte VDD (Bandgap) em mV"));
            Serial.println(F("reset        - Zera o buffer de amostras e reinicia o indice"));
            Serial.println(F("help         - Exibe esta mensagem de ajuda"));
            Serial.println(F("----------------------------"));
        }
        // Comando C<n> (ex: C4 ou c4)
        else if (cmd.startsWith("C") || cmd.startsWith("c"))
        {
            int n = cmd.substring(1).toInt();
            if (n >= 1 && n <= MAX_ADC_CHANNELS)
            {
                u8_numCanais = (uint8_t)n;
                alocarBufferContinuo();
                Serial.print(F(">> Numero de entradas alterado para: "));
                Serial.println(u8_numCanais);
            }
            else
            {
                Serial.print(F(">> Erro: O numero de entradas deve ser entre 1 e "));
                Serial.println(MAX_ADC_CHANNELS);
            }
        }
        // Comando TU<ms> (ex: TU3000)
        else if (cmd.startsWith("TU") || cmd.startsWith("tu"))
        {
            unsigned long tu = cmd.substring(2).toInt();
            if (tu >= ul_tempoAquisicao_ms)
            {
                ul_tempoEnvio_ms = tu;
                alocarBufferContinuo();
                Serial.print(F(">> Tempo de envio UART alterado para: "));
                Serial.print(ul_tempoEnvio_ms);
                Serial.println(F(" ms"));
            }
            else
            {
                Serial.println(F(">> Erro: Tempo UART deve ser maior ou igual ao tempo de aquisicao."));
            }
        }
        // Comando TA<ms> (ex: TA500)
        else if (cmd.startsWith("TA") || cmd.startsWith("ta"))
        {
            unsigned long ta = cmd.substring(2).toInt();
            if (ta >= 50 && ta <= ul_tempoEnvio_ms)
            {
                ul_tempoAquisicao_ms = ta;
                alocarBufferContinuo();
                Serial.print(F(">> Tempo de aquisicao alterado para: "));
                Serial.print(ul_tempoAquisicao_ms);
                Serial.println(F(" ms"));
            }
            else
            {
                Serial.println(F(">> Erro: Tempo de aquisicao invalido (min 50 ms)."));
            }
        }
        // Comando combinado no formato CSV: <canais>,<tempo_uart> OU <canais>,<tempo_aq>,<tempo_uart>
        else if (cmd.indexOf(',') > 0)
        {
            int c1 = cmd.indexOf(',');
            int c2 = cmd.indexOf(',', c1 + 1);

            int canais = 0;
            unsigned long ta = ul_tempoAquisicao_ms;
            unsigned long tu = ul_tempoEnvio_ms;

            if (c2 > 0) // 3 parâmetros: canais,tempo_aquisicao,tempo_uart
            {
                canais = cmd.substring(0, c1).toInt();
                ta = cmd.substring(c1 + 1, c2).toInt();
                tu = cmd.substring(c2 + 1).toInt();
            }
            else // 2 parâmetros: canais,tempo_uart
            {
                canais = cmd.substring(0, c1).toInt();
                tu = cmd.substring(c1 + 1).toInt();
            }

            if (canais >= 1 && canais <= MAX_ADC_CHANNELS && ta >= 50 && tu >= ta)
            {
                u8_numCanais = (uint8_t)canais;
                ul_tempoAquisicao_ms = ta;
                ul_tempoEnvio_ms = tu;
                alocarBufferContinuo();
                Serial.print(F(">> Configuracao atualizada com sucesso: "));
                Serial.print(u8_numCanais);
                Serial.print(F(" entradas | Tempo Aquisicao: "));
                Serial.print(ul_tempoAquisicao_ms);
                Serial.print(F(" ms | Tempo UART: "));
                Serial.print(ul_tempoEnvio_ms);
                Serial.println(F(" ms."));
            }
            else
            {
                Serial.println(F(">> Formato ou valores invalidos. Use: <canais>,<tempo_uart> OU <canais>,<tempo_aq>,<tempo_uart> (ex: 4,250,5000)"));
            }
        }
        else if (cmd.equalsIgnoreCase("read"))
        {
            Serial.print(F(">> Leituras Instantaneas (mV): "));
            for (uint8_t ch = 0; ch < u8_numCanais; ch++)
            {
                int16_t raw = analogRead(A0 + ch);
                Serial.print(F("A"));
                Serial.print(ch);
                Serial.print(F(": "));
                Serial.print(i16_convertADC(raw));
                Serial.print(F(" mV"));
                if (ch < u8_numCanais - 1) Serial.print(F(" | "));
            }
            Serial.println();
        }
        else if (cmd.equalsIgnoreCase("send"))
        {
            transmitirLoteUART();
        }
        else if (cmd.equalsIgnoreCase("vdd"))
        {
            Serial.print(F(">> VDD Calibrado (Alimentacao): "));
            Serial.print(i16_VDDGlobal);
            Serial.println(F(" mV"));
        }
        else if (cmd.equalsIgnoreCase("reset"))
        {
            size_t totalBytes = sizeof(int16_t) * u16_elementosPorAmostra * u16_totalAmostras;
            memset(i16_adcData, 0, totalBytes);
            u16_indiceAmostra = 0;
            Serial.println(F(">> Buffer de telemetria reinicializado para zero."));
        }
        else if (cmd.length() > 0)
        {
            Serial.print(F(">> Comando desconhecido: "));
            Serial.print(cmd);
            Serial.println(F(". Digite 'help' para opcoes."));
        }
    }
}

// ============================================================
// Inicialização do Sistema (Setup)
// ============================================================
void setup()
{
    Serial.begin(9600);
    pinMode(LED_BUILTIN, OUTPUT);

    Serial.println(F("\n--- INICIALIZANDO DATA LOGGER FINAL (AULA 7) ---"));

    // 1. Aloca dinamicamente o buffer contínuo de amostras na Heap
    alocarBufferContinuo();

    // 2. Calibra o VDD inicial via Bandgap
    i16_VDDGlobal = i16_calcVDD();
    Serial.print(F("[INFO] VDD inicial calibrado: "));
    Serial.print(i16_VDDGlobal);
    Serial.println(F(" mV"));

    // 3. Inicializa os temporizadores
    unsigned long agora = millis();
    ul_timerUpdateVDD = agora;
    ul_timerADCAQ     = agora;
    ul_timerAQSEND    = agora;

    Serial.println(F("[INFO] Modo de operacao: Todas as entradas sao convertidas e exibidas estritamente em milivolts (mV)."));
    Serial.println(F("[INFO] Sistema pronto. Digite 'help' no Monitor Serial."));
}

// ============================================================
// Loop Principal Não-Bloqueante
// ============================================================
void loop()
{
    unsigned long ul_tempoAtual = millis();

    // 1. Processamento de comandos interativos pela Serial
    processarComandosUART();

    // 2. Calibração periódica da tensão de alimentação (VDD) a cada 1 s
    if (ul_tempoAtual - ul_timerUpdateVDD >= TEMPO_VDD_MS)
    {
        ul_timerUpdateVDD = ul_tempoAtual;
        i16_VDDGlobal = i16_calcVDD();
    }

    // 3. Aquisição periódica de canais ADC (taxa configurável: ul_tempoAquisicao_ms)
    if (ul_tempoAtual - ul_timerADCAQ >= ul_tempoAquisicao_ms)
    {
        ul_timerADCAQ = ul_tempoAtual;

        // Toggle do LED indicador de amostragem ativa (Heartbeat)
        digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));

        // Calcula o offset no buffer para a amostra atual
        uint16_t u16_offsetAmostra = u16_indiceAmostra * u16_elementosPorAmostra;

        // Leitura e conversão dos canais analógicos ativos em mV
        for (uint8_t ch = 0; ch < u8_numCanais; ch++)
        {
            int16_t i16_raw = analogRead(A0 + ch);
            i16_adcData[u16_offsetAmostra + ch] = i16_convertADC(i16_raw);
        }

        // Calcula o CRC-16 cobrindo exclusivamente os canais analógicos desta amostra
        uint16_t u16_crcCalculado = crc16_modbus(
            (const uint8_t *)&i16_adcData[u16_offsetAmostra],
            sizeof(int16_t) * u8_numCanais
        );

        // Salva o CRC-16 no último slot da amostra
        i16_adcData[u16_offsetAmostra + u8_numCanais] = (int16_t)u16_crcCalculado;

        // [OPCIONAL] Descomente o bloco abaixo caso queira exibir cada leitura individual em tempo real na tela:
        /*
        Serial.print(F("AQ#"));
        if (u16_indiceAmostra < 9) Serial.print(F("0"));
        Serial.print(u16_indiceAmostra + 1);
        Serial.print(F("/"));
        Serial.print(u16_totalAmostras);
        Serial.print(F(" | "));

        for (uint8_t ch = 0; ch < u8_numCanais; ch++)
        {
            Serial.print(F("A"));
            Serial.print(ch);
            Serial.print(F(": "));
            Serial.print(i16_adcData[u16_offsetAmostra + ch]);
            Serial.print(F(" mV"));
            if (ch < u8_numCanais - 1)
            {
                Serial.print(F(" | "));
            }
        }
        Serial.print(F(" | CRC: 0x"));
        if (u16_crcCalculado < 0x1000) Serial.print(F("0"));
        Serial.print(u16_crcCalculado, HEX);
        Serial.println();
        */

        // Avança circularmente o índice de amostras
        u16_indiceAmostra = (u16_indiceAmostra + 1) % u16_totalAmostras;
    }

    // 4. Transmissão periódica de dados via UART (intervalo configurável: ul_tempoEnvio_ms)
    if (ul_tempoAtual - ul_timerAQSEND >= ul_tempoEnvio_ms)
    {
        ul_timerAQSEND = ul_tempoAtual;
        transmitirLoteUART();
    }
}
