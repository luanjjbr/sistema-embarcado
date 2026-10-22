/*================================================================================
          ANÁLISE, CORREÇÃO E DESCRIÇÃO DETALHADA - CÓDIGO AULA 1
                    SISTEMA EMBARCADO (DATA LOGGER)
================================================================================

1. PROBLEMAS E ERROS IDENTIFICADOS NO CÓDIGO ORIGINAL
--------------------------------------------------------------------------------

1.1. Incompatibilidade Crítica de Memória (Union Incompatível):
     * No código original, a union unia um array `int16_t i16_dados[9]` (18 bytes) 
       com uma struct formada por campos `int32_t` (36 bytes).
     * Como a union compartilha o mesmo espaço físico de memória, ao escrever em um
       campo de 32 bits (`int32_t`), você sobrepunha/corrompia dois elementos de 
       16 bits (`int16_t`) do array.
     * Correção: Todos os membros da struct dentro da union foram alterados para 
       `int16_t`, alinhando perfeitamente a representação em array (indexada) com 
       a representação por nomes (estritamente 18 bytes).

1.2. Condição de Loop na Função `move()`:
     * A variável de controle `u8_i` era `uint8_t` (sem sinal) e decrementava até 0. 
       Ao checar `u8_i > 0`, o índice 0 nunca era atingido de forma segura e o uso 
       de tipos sem sinal em loops decrescentes pode gerar bugs de estouro (underflow).
     * Correção: Ajustado o tipo para `int8_t` garantindo um controle seguro do loop.

1.3. Conversão de Tipos na Comparação de Tempo:
     * A expressão `Log.f32_tempo_aquisicao * 1000` gera um valor de ponto flutuante (`float`). 
       Comparar diretamente `uint32_t` com `float` força conversões implícitas em todo loop,
       o que consome ciclos de CPU e memória em microcontroladores sem FPU dedicada.
     * Correção: Aplicado *cast* explícito `(uint32_t)` para a conversão do tempo base em milissegundos.


2. DESCRIÇÃO MÓDULO POR MÓDULO DO CÓDIGO CORRIGIDO
--------------------------------------------------------------------------------

2.1. Estrutura de Dados em União (`union _st_data` / `st_data`):
     - Finalidade: Permite acessar os mesmos dados de duas formas distintas sem ocupar 
       memória extra:
       a) Como array (`i16_dados[9]`): ideal para laços de repetição `for` ao ler canais.
       b) Como estrutura (`st_adc`): ideal para acesso nomeado (ex: `i16_Time`, `i16_adc0`).
     - Alinhamento: 9 elementos × 2 bytes = 18 bytes totais.

2.2. Estrutura do Data Logger (`st_data_logger_t`):
     - `u8_canais`: Quantidade de canais ADC ativos para leitura.
     - `f32_tempo_aquisicao`: Intervalo de tempo entre amostragens (em segundos).
     - `f32_tempo_transferencia`: Intervalo de tempo entre os envios via UART (em segundos).
     - `st_dado[8]`: Buffer de memória que armazena até 8 amostras de dados históricas.

2.3. Função `move(st_data st_dado[8])`:
     - Finalidade: Implementa o deslocamento do histórico de amostras.
     - Funcionamento: Move cada registro uma posição para trás no array (a amostra mais antiga
       em `st_dado[7]` é descartada e a posição `st_dado[0]` fica livre para o novo registro).

2.4. Função `print_data(st_data st_dado[8], uint8_t u8_canais)`:
     - Finalidade: Formata e envia os dados armazenados para a porta serial (UART).
     - Funcionamento: Varre o histórico e imprime os valores em formato CSV (valores separados 
       por vírgula) terminando cada registro com uma quebra de linha.

2.5. Função `uart_config()`:
     - Finalidade: Efetua a leitura e esvaziamento do buffer de recepção da UART quando 
       há caracteres disponíveis.

2.6. Loop Principal (`loop()`):
     - Temporização Não-Bloqueante: Utiliza a função `millis()` para gerenciar dois temporizadores 
       independentes sem travar o processador com funções como `delay()`:
       1) Temporizador de Aquisição: Desloca o histórico, lê os pinos analógicos e salva 
          os dados na posição `st_dado[0]`.
       2) Temporizador de Transferência: Dispara a impressão serial dos dados acumulados.


================================================================================
                      CÓDIGO-FONTE C/C++ CORRIGIDO
================================================================================
*/
#include <Arduino.h>

// Estrutura dos dados (União com alinhamento correto de 18 bytes)
typedef union _st_data
{
    int16_t i16_dados[9];

    struct _st_adc {
        int16_t i16_Time;
        int16_t i16_adc0;
        int16_t i16_adc1;
        int16_t i16_adc2;
        int16_t i16_adc3;
        int16_t i16_adc4;
        int16_t i16_adc5;
        int16_t i16_adc6;
        int16_t i16_adc7;
    } st_adc;

} st_data;

// Estrutura do Data Logger
typedef struct _st_data_logger_t
{
    uint8_t u8_canais;
    float f32_tempo_aquisicao;
    float f32_tempo_transferencia;
    st_data st_dado[8];
} st_data_logger_t;

// Instância do Data Logger
static st_data_logger_t Log = {2, 1.0f, 5.0f, {{{0}}}};

// Temporizadores
uint32_t u32_tempo_aquisicao = 0;
uint32_t u32_tempo_uart = 0;

// Limites
static const uint8_t u8_min_canais = 1;
static const uint8_t u8_max_canais = 8;

// Deslocamento do buffer
void move(st_data st_dado[8])
{
    for (int8_t i8_i = 7; i8_i > 0; i8_i--)
    {
        st_dado[i8_i] = st_dado[i8_i - 1];
    }
}

// Envio dos dados via UART
void print_data(st_data st_dado[8], uint8_t u8_canais)
{
    for (uint8_t u8_i = 0; u8_i < 8; u8_i++)
    {
        for (uint8_t u8_j = 0; u8_j <= u8_canais; u8_j++)
        {
            Serial.print(st_dado[u8_i].i16_dados[u8_j]);
            if (u8_j < u8_canais)
            {
                Serial.print(",");
            }
        }
        Serial.print("\n");
    }
    Serial.print("---------------------------\n");
}

// Leitura da Serial
void uart_config()
{
    while (Serial.available() > 0)
    {
        char c_caracter = Serial.read();
        if (c_caracter == '\n' || c_caracter == '\r')
        {
            Serial.println(c_caracter);
        }
    }
}

void setup()
{
    Serial.begin(9600);
    u32_tempo_aquisicao = millis();
    u32_tempo_uart = millis();
}

void loop()
{
    uint32_t u32_tempo_atual = millis();

    // Aquisição periódica de dados
    if ((u32_tempo_atual - u32_tempo_aquisicao) >= (uint32_t)(Log.f32_tempo_aquisicao * 1000.0f))
    {
        move(Log.st_dado);
        Log.st_dado[0].st_adc.i16_Time++;

        for (uint8_t u8_i = 0; u8_i < Log.u8_canais && u8_i < u8_max_canais; u8_i++)
        {
            Log.st_dado[0].i16_dados[u8_i + 1] = analogRead(u8_i);
        }

        u32_tempo_aquisicao = u32_tempo_atual;
    }

    // Transmissão periódica via UART
    if ((u32_tempo_atual - u32_tempo_uart) >= (uint32_t)(Log.f32_tempo_transferencia * 1000.0f))
    {
        print_data(Log.st_dado, Log.u8_canais);
        u32_tempo_uart = u32_tempo_atual;
    }

    uart_config();
}