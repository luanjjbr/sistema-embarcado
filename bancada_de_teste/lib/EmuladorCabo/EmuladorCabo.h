/**
 * ============================================================================
 * Classe: EmuladorCabo
 * Arquivo: EmuladorCabo.h
 * Finalidade: Abstração do Emulador do Cabo de Recarga (Pino PP) em C++ (POO)
 * Referência: IEC 62196-2 / IEC 61851-1 (Artigo UFSM - Rosa et al.)
 * ============================================================================
 */

#ifndef EMULADOR_CABO_H
#define EMULADOR_CABO_H

#include <Arduino.h>

class EmuladorCabo {
private:
    // Pinos digitais conectados aos gates dos MOSFETs S1 a S6
    uint8_t _pinos[6];

    // Canal atualmente ativo:
    // 0 = Nenhum (Aberto total, estado padrão)
    // 1 = S1 (4700R), 2 = S2 (1500R), 3 = S3 (680R), 
    // 4 = S4 (220R),  5 = S5 (100R),  6 = S6 (47R)
    int _canalAtivo;

public:
    /**
     * @brief Construtor: associa os 6 pinos do microcontrolador ao objeto
     */
    EmuladorCabo(uint8_t pinoS1, uint8_t pinoS2, uint8_t pinoS3,
                 uint8_t pinoS4, uint8_t pinoS5, uint8_t pinoS6);

    /**
     * @brief Configura todos os pinos como OUTPUT e garante todos em nível LOW
     */
    void begin();

    /**
     * @brief Desliga todos os MOSFETs (Retorna ao estado padrão de circuito aberto)
     */
    void desligarTodos();

    /**
     * @brief Ativa exclusivamente um canal com intertravamento Break-Before-Make
     * @param canal Número do canal (0 a 6)
     * @return true se o comando foi aceito
     */
    bool ativarCanal(int canal);

    /**
     * @brief Retorna qual canal está ativo no momento (0 a 6)
     */
    int getCanalAtivo() const;

    /**
     * @brief Retorna a resistência nominal emulada em Ohms
     */
    uint16_t getResistencia() const;

    /**
     * @brief Retorna a capacidade de corrente máxima em Amperes
     */
    int getCorrenteMaxima() const;

    /**
     * @brief Retorna a descrição do estado atual
     */
    const char* getDescricao() const;

    /**
     * @brief Imprime relatório visual detalhado na Serial
     */
    void imprimirStatus() const;
};

#endif // EMULADOR_CABO_H

