#include <Arduino.h>
#include <stdlib.h>
#include <string.h> // Necessário para memset

#define MAX_SAMPLES 20

// Estrutura para a amostra na memória dinâmica
typedef struct _stAQData {
    int16_t *i16_vectADC; // Vetor para os canais em mV
    uint16_t ui16_crc;    // Checksum CRC-16 do vetor de dados
} stAQData;

// Estrutura do Data Logger
typedef struct _stADC {
    int16_t i16_nchan;            // Número de canais ativos (1 a 8)
    uint32_t ui32_tempo_aquisicao;// Tempo de amostragem (ms)
    uint32_t ui32_tempo_uart;     // Tempo de envio UART (ms)
    stAQData *s_adcData;          // Buffer dinâmico de amostras
} stADC;

// Instância global com valores padrão
stADC stADC0 = {
    .i16_nchan = 4, 
    .ui32_tempo_aquisicao = 1000, 
    .ui32_tempo_uart = 5000, 
    .s_adcData = NULL
};

uint32_t ui32_Time_VCC = 0;
uint32_t ui32_Time_ADCQ = 0;
uint32_t ui32_Time_UART = 0;

int32_t i32_vcc_atual_mV = 5000;
uint16_t u16_indice_amostra = 0;

// Prototipação
int16_t i16_readBandGap();
int32_t i32_calcularVCC();
uint16_t calcularCRC16(const uint8_t *data, size_t length);
void alocarEMemoriaZerar();
void liberarMemoria();
void verificarComandosUART();

// ============================================================
// Algoritmo CRC-16 (Modbus - Polinômio 0xA001)
// ============================================================
uint16_t calcularCRC16(const uint8_t *data, size_t length) {
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < length; i++) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}

// ============================================================
// Gestão de Memória: Alocação e Limpeza Total (Reset para 0)
// ============================================================
void alocarEMemoriaZerar() {
    // Aloca memória para o array de amostras
    stADC0.s_adcData = (stAQData *) malloc(MAX_SAMPLES * sizeof(stAQData));

    if (stADC0.s_adcData != NULL) {
        // Zera toda a estrutura stAQData principal (reseta ponteiros e CRC)
        memset(stADC0.s_adcData, 0, MAX_SAMPLES * sizeof(stAQData));

        for (uint16_t u16_k = 0; u16_k < MAX_SAMPLES; u16_k++) {
            // Aloca o vetor dinâmico do ADC para cada amostra
            stADC0.s_adcData[u16_k].i16_vectADC = (int16_t *) malloc(sizeof(int16_t) * stADC0.i16_nchan);
            
            // Zera todos os bytes do vetor do ADC
            if (stADC0.s_adcData[u16_k].i16_vectADC != NULL) {
                memset(stADC0.s_adcData[u16_k].i16_vectADC, 0, sizeof(int16_t) * stADC0.i16_nchan);
            }
        }
    }
}

void liberarMemoria() {
    if (stADC0.s_adcData != NULL) {
        for (uint16_t u16_k = 0; u16_k < MAX_SAMPLES; u16_k++) {
            if (stADC0.s_adcData[u16_k].i16_vectADC != NULL) {
                free(stADC0.s_adcData[u16_k].i16_vectADC);
            }
        }
        free(stADC0.s_adcData);
        stADC0.s_adcData = NULL;
    }
}

// ============================================================
// Parser de Interação Serial (UART)
// ============================================================
void verificarComandosUART() {
    if (Serial.available() > 0) {
        String cmd = Serial.readStringUntil('\n');
        cmd.trim();

        // Altera quantidade de canais, desaloca e realoca tudo zerado (ex: "C4")
        if (cmd.startsWith("C") || cmd.startsWith("c")) {
            int16_t novos_canais = cmd.substring(1).toInt();
            if (novos_canais >= 1 && novos_canais <= 8) {
                liberarMemoria();
                stADC0.i16_nchan = novos_canais;
                alocarEMemoriaZerar(); // Realoca e limpa tudo para 0 novamente
                u16_indice_amostra = 0;
                
                Serial.print(">> Memoria zerada! Canais definidos para: ");
                Serial.println(stADC0.i16_nchan);
            } else {
                Serial.println(">> Erro: Canais devem ser entre 1 e 8.");
            }
        }
        // Altera tempo de aquisição em ms (ex: "TA500")
        else if (cmd.startsWith("TA") || cmd.startsWith("ta")) {
            uint32_t novo_tempo = cmd.substring(2).toInt();
            if (novo_tempo >= 100) {
                stADC0.ui32_tempo_aquisicao = novo_tempo;
                Serial.print(">> Tempo de Aquisicao: ");
                Serial.print(stADC0.ui32_tempo_aquisicao);
                Serial.println(" ms");
            }
        }
        // Altera tempo de transmissão UART em ms (ex: "TU2000")
        else if (cmd.startsWith("TU") || cmd.startsWith("tu")) {
            uint32_t novo_tempo = cmd.substring(2).toInt();
            if (novo_tempo >= 500) {
                stADC0.ui32_tempo_uart = novo_tempo;
                Serial.print(">> Tempo UART: ");
                Serial.print(stADC0.ui32_tempo_uart);
                Serial.println(" ms");
            }
        }
    }
}

// ============================================================
// Setup e Loop
// ============================================================
void setup() {
    Serial.begin(9600);
    while (!Serial);

    // Aloca e garante que toda a RAM utilizada inicie puramente com o valor 0
    alocarEMemoriaZerar();
    
    i32_vcc_atual_mV = i32_calcularVCC();

    Serial.println("--- Memoria RAM zerada com sucesso ---");
    Serial.println("System Ready: Digite comandos (Ex: C4, TA500, TU2000)");
}

void loop() {
    uint32_t u32_tempo_atual = millis();

    verificarComandosUART();

    // 1. Recalibração do VCC via Bandgap
    if (u32_tempo_atual - ui32_Time_VCC >= 2000) {
        ui32_Time_VCC = u32_tempo_atual;
        i32_vcc_atual_mV = i32_calcularVCC();
    }

    // 2. Aquisição de Dados + Cálculo do CRC
    if (u32_tempo_atual - ui32_Time_ADCQ >= stADC0.ui32_tempo_aquisicao) {
        ui32_Time_ADCQ = u32_tempo_atual;

        if (stADC0.s_adcData != NULL && stADC0.s_adcData[u16_indice_amostra].i16_vectADC != NULL) {
            for (int16_t ch = 0; ch < stADC0.i16_nchan; ch++) {
                int16_t i16_raw_adc = analogRead(ch);
                stADC0.s_adcData[u16_indice_amostra].i16_vectADC[ch] = (int16_t)(((int32_t)i16_raw_adc * i32_vcc_atual_mV) / 1023L);
            }

            size_t tamanho_bytes = sizeof(int16_t) * stADC0.i16_nchan;
            stADC0.s_adcData[u16_indice_amostra].ui16_crc = calcularCRC16(
                (const uint8_t *)stADC0.s_adcData[u16_indice_amostra].i16_vectADC, 
                tamanho_bytes
            );

            u16_indice_amostra = (u16_indice_amostra + 1) % MAX_SAMPLES;
        }
    }

    // 3. Transmissão UART
    if (u32_tempo_atual - ui32_Time_UART >= stADC0.ui32_tempo_uart) {
        ui32_Time_UART = u32_tempo_atual;

        Serial.println("\n--- Relatorio do Data Logger (mV) ---");
        for (uint16_t k = 0; k < MAX_SAMPLES; k++) {
            uint16_t idx = (u16_indice_amostra + k) % MAX_SAMPLES;
            
            Serial.print("Amostra "); Serial.print(k); Serial.print(": ");
            for (int16_t ch = 0; ch < stADC0.i16_nchan; ch++) {
                Serial.print(stADC0.s_adcData[idx].i16_vectADC[ch]);
                Serial.print("mV, ");
            }
            Serial.print("| CRC16: 0x");
            if (stADC0.s_adcData[idx].ui16_crc < 0x1000) Serial.print("0");
            Serial.println(stADC0.s_adcData[idx].ui16_crc, HEX);
        }
        Serial.print("VCC Atual: "); Serial.print(i32_vcc_atual_mV); Serial.println(" mV");
        Serial.println("-------------------------------------");
    }
}

// ============================================================
// Leitura VCC (Bandgap)
// ============================================================
int16_t i16_readBandGap() {
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
    delay(2);
    ADCSRA |= _BV(ADSC);
    while (bit_is_set(ADCSRA, ADSC));
    return ADCW;
}

int32_t i32_calcularVCC() {
    int16_t i16_bg = i16_readBandGap();
    if (i16_bg <= 0) return 5000;
    return (1023L * 1100L) / (int32_t)i16_bg;
}