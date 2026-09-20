/**
 * ============================================================================
 * Projeto: Bancada
 * Arquivo: EmuladorCabo.h
 * Finalidade: Abstração do Emulador do Cabo de Recarga (Pino PP - IEC 62196-2)
 * Microcontroladores: Arduino Nano (ATmega328P) / Arduino Mega 2560
 * ============================================================================
 */

#ifndef EMULADOR_CABO_H
#define EMULADOR_CABO_H

#include <Arduino.h>

/**
 * @brief Tipos de estado e capacidades de corrente do cabo (Pino PP)
 */
enum class TipoCabo : uint8_t {
    ABERTO        = 0,  // Todas as chaves desligadas (sem conexão)
    DESCONECTADO  = 1,  // S1: 4700 Ohms (Desconectado nominal)
    CABO_13A      = 2,  // S2: 1500 Ohms (Cabo 13 A)
    CABO_20A      = 3,  // S3:  680 Ohms (Cabo 20 A)
    CABO_32A      = 4,  // S4:  220 Ohms (Cabo 32 A)
    CABO_63A      = 5,  // S5:  100 Ohms (Cabo 63 A / 70 A)
    FALHA_CURTO   = 6   // S6:   47 Ohms (Simulação de Curto / Falha)
};

class EmuladorCabo {
private:
    uint8_t _pinos[6];       // Pinos digitais conectados aos gates dos MOSFETs S1 a S6
    TipoCabo _tipoAtivo;     // Estado ativo no momento

public:
    /**
     * @brief Construtor: registra os 6 pinos associados às chaves S1 a S6
     */
    EmuladorCabo(uint8_t pinoS1, uint8_t pinoS2, uint8_t pinoS3,
                 uint8_t pinoS4, uint8_t pinoS5, uint8_t pinoS6);

    /**
     * @brief Configura todos os pinos como OUTPUT e garante todos em LOW
     */
    void begin();

    /**
     * @brief Desliga todos os MOSFETs (Retorna ao estado Aberto Total)
     */
    void desligarTodos();

    /**
     * @brief Ativa um canal específico utilizando o enum TipoCabo
     * @param tipo Estado do cabo a ser selecionado
     * @return true se a ativação foi executada com sucesso
     */
    bool ativarCanal(TipoCabo tipo);

    /**
     * @brief Sobrecarga para ativar por número inteiro (0 a 6)
     */
    bool ativarCanal(int canal);

    /**
     * @brief Retorna o enum do tipo de cabo ativo
     */
    TipoCabo getTipoAtivo() const;

    /**
     * @brief Retorna o número inteiro do canal ativo (0 a 6)
     */
    int getCanalAtivo() const;

    /**
     * @brief Retorna o valor nominal de resistência em Ohms (0 se aberto)
     */
    uint16_t getResistencia() const;

    /**
     * @brief Retorna a capacidade máxima de corrente em Amperes (0, 13, 20, 32, 63)
     */
    int getCorrenteMaxima() const;

    /**
     * @brief Retorna uma descrição em texto do estado atual
     */
    const char* getDescricao() const;

    /**
     * @brief Imprime relatório formatado na porta Serial
     */
    void imprimirStatus() const;
};

#endif // EMULADOR_CABO_H
