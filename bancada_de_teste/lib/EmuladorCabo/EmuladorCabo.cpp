/**
 * ============================================================================
 * Implementação: EmuladorCabo
 * Arquivo: EmuladorCabo.cpp
 * ============================================================================
 */

#include "EmuladorCabo.h"

EmuladorCabo::EmuladorCabo(uint8_t pinoS1, uint8_t pinoS2, uint8_t pinoS3,
                           uint8_t pinoS4, uint8_t pinoS5, uint8_t pinoS6) {
    _pinos[0] = pinoS1;
    _pinos[1] = pinoS2;
    _pinos[2] = pinoS3;
    _pinos[3] = pinoS4;
    _pinos[4] = pinoS5;
    _pinos[5] = pinoS6;
    _canalAtivo = 0;
}

void EmuladorCabo::begin() {
    for (int i = 0; i < 6; i++) {
        pinMode(_pinos[i], OUTPUT);
        digitalWrite(_pinos[i], LOW);
    }
    _canalAtivo = 0;
}

void EmuladorCabo::desligarTodos() {
    for (int i = 0; i < 6; i++) {
        digitalWrite(_pinos[i], LOW);
    }
    _canalAtivo = 0;
}

bool EmuladorCabo::ativarCanal(int canal) {
    // 1. REGRA DE SEGURANÇA: Desliga todos os pinos primeiro (Break-Before-Make)
    // Isso impede fisicamente que duas resistências fiquem em paralelo!
    desligarTodos();

    // Se o canal solicitado for 0, o objetivo era apenas desligar tudo (Aberto total)
    if (canal == 0) {
        return true;
    }

    // Se for um canal válido entre 1 e 6, aciona exclusivamente o pino correspondente
    if (canal >= 1 && canal <= 6) {
        digitalWrite(_pinos[canal - 1], HIGH);
        _canalAtivo = canal;
        return true;
    }

    return false; // Canal inválido
}

int EmuladorCabo::getCanalAtivo() const {
    return _canalAtivo;
}

uint16_t EmuladorCabo::getResistencia() const {
    switch (_canalAtivo) {
        case 1: return 4700; // S1
        case 2: return 1500; // S2
        case 3: return 680;  // S3
        case 4: return 220;  // S4
        case 5: return 100;  // S5
        case 6: return 47;   // S6
        default: return 0;   // Aberto / Infinito
    }
}

int EmuladorCabo::getCorrenteMaxima() const {
    switch (_canalAtivo) {
        case 2: return 13; // S2 -> 13 A
        case 3: return 20; // S3 -> 20 A
        case 4: return 32; // S4 -> 32 A
        case 5: return 63; // S5 -> 63 A
        default: return 0; // Desconectado ou Falha
    }
}

const char* EmuladorCabo::getDescricao() const {
    switch (_canalAtivo) {
        case 0: return "PADRAO (Aberto Total / Nenhuma chave ligada)";
        case 1: return "S1 ATIVA: 4700 Ohms (Desconectado nominal)";
        case 2: return "S2 ATIVA: 1500 Ohms (Cabo de 13 A)";
        case 3: return "S3 ATIVA:  680 Ohms (Cabo de 20 A)";
        case 4: return "S4 ATIVA:  220 Ohms (Cabo de 32 A)";
        case 5: return "S5 ATIVA:  100 Ohms (Cabo de 63 A / 70 A)";
        case 6: return "S6 ATIVA:   47 Ohms (Falha / Curto-circuito)";
        default: return "DESCONHECIDO";
    }
}

void EmuladorCabo::imprimirStatus() const {
    Serial.println(F("--------------------------------------------------"));
    Serial.print(F(" [EMULADOR PP] Estado Atual: "));
    Serial.println(getDescricao());

    if (_canalAtivo > 0) {
        Serial.print(F(" Resistencia PP-PE:  "));
        Serial.print(getResistencia());
        Serial.println(F(" Ohms"));

        Serial.print(F(" Corrente do Cabo:   "));
        Serial.print(getCorrenteMaxima());
        Serial.println(F(" A"));

        Serial.print(F(" Pino Digital Ativo: "));
        Serial.println(_pinos[_canalAtivo - 1]);
    } else {
        Serial.println(F(" Resistencia PP-PE:  Infinita (Circuito Aberto)"));
        Serial.println(F(" Corrente do Cabo:   0 A (Inoperante)"));
        Serial.println(F(" Pinos Digitais:     Todos em nivel LOW (0V)"));
    }
    Serial.println(F("--------------------------------------------------"));
}

