#include <Arduino.h>
#include <stdlib.h> // malloc e free

#define DEF_NUM_ADC_CHANNELS 10

// Estrutura de nó para Lista Encadeada de Amostras
typedef struct _stAQData {
    int16_t *i16_vectADC;      // Ponteiro para o array dinâmico de canais
    uint16_t u16_crc;          // Checksum dos dados
    struct _stAQData *next;    // Ponteiro para a próxima amostra (corrigido typo 'nex')
} stAQData;

typedef struct _stADC {
    int16_t i16_nchan;         // Número de canais ativos
    int32_t i32_tAQ;           // Tempo de amostragem
    stAQData *s_adcData;       // Ponteiro para a cabeça da lista de amostras
} stADC;

// Variáveis Globais
int16_t *it16_adcdata = NULL;
int16_t i16_adcdataleng;

void setup() {
    Serial.begin(9600);
    while (!Serial); // Aguarda inicialização da Serial (para placas como Pico/Micro)

    i16_adcdataleng = DEF_NUM_ADC_CHANNELS;

    // Alocação dinâmica de memória usando sizeof
    it16_adcdata = (int16_t *) malloc(sizeof(int16_t) * i16_adcdataleng);

    // Validação correta: malloc retorna NULL em caso de erro
    if (it16_adcdata == NULL) {
        Serial.println("Erro: Memoria insuficiente (malloc falhou)!");
    } 
    else {
        Serial.println("Alocacao de memoria OK!");

        // Preenche o buffer alocado com valores simulados
        for (int16_t u16_idx = 0; u16_idx < i16_adcdataleng; u16_idx++) {
            it16_adcdata[u16_idx] = (int16_t) rand() % 1024; // Simula leitura ADC (0-1023)
        }

        // Exibe os valores alocados no Monitor Serial
        for (int16_t u16_idx = 0; u16_idx < i16_adcdataleng; u16_idx++) {
            Serial.print("Canal ");
            Serial.print(u16_idx);
            Serial.print(": ");
            Serial.println(it16_adcdata[u16_idx]);
        }

        // Libera a memória após o uso para evitar Memory Leak
        free(it16_adcdata);
        it16_adcdata = NULL; // Evita ponteiro solto (dangling pointer)
    }
}

void loop() {
    // Código de loop principal
}