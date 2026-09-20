/**
 * ============================================================================
 * Projeto: Bancada
 * Arquivo: EmuladorCabo.cpp
 * Finalidade: Implementação da classe EmuladorCabo com Break-Before-Make
 * Microcontroladores: Arduino Nano (ATmega328P) / Arduino Mega 2560
 * ============================================================================
 */

#include "EmuladorCabo.h"

EmuladorCabo::EmuladorCabo(uint8_t pinoS1, uint8_t pinoS2, uint8_t pinoS3,
                           uint8_t pinoS4, uint8_t pinoS5, uint8_t pinoS6)
    : _tipoAtivo(TipoCabo::ABERTO)
{
    _pinos[0] = pinoS1;
    _pinos[1] = pinoS2;
    _pinos[2] = pinoS3;
    _pinos[3] = pinoS4;
    _pinos[4] = pinoS5;
    _pinos[5] = pinoS6;
}

void EmuladorCabo::begin()
{
    for (uint8_t i = 0; i < 6; i++)
    {
        pinMode(_pinos[i], OUTPUT);
        digitalWrite(_pinos[i], LOW);
    }
    _tipoAtivo = TipoCabo::ABERTO;
}

void EmuladorCabo::desligarTodos()
{
    for (uint8_t i = 0; i < 6; i++)
    {
        digitalWrite(_pinos[i], LOW);
    }
    _tipoAtivo = TipoCabo::ABERTO;
}

bool EmuladorCabo::ativarCanal(TipoCabo tipo)
{
    // 1. Intertravamento BREAK: Desliga fisicamente todas as chaves
    desligarTodos();

    // 2. Se for estado ABERTO, encerra aqui (todas já estão LOW)
    if (tipo == TipoCabo::ABERTO)
    {
        return true;
    }

    // 3. Intertravamento MAKE: Liga apenas a chave do canal selecionado
    uint8_t indicePino = static_cast<uint8_t>(tipo) - 1;

    if (indicePino < 6)
    {
        digitalWrite(_pinos[indicePino], HIGH);
        _tipoAtivo = tipo;
        return true;
    }

    return false;
}

bool EmuladorCabo::ativarCanal(int canal)
{
    if (canal >= 0 && canal <= 6)
    {
        return ativarCanal(static_cast<TipoCabo>(canal));
    }
    return false;
}

TipoCabo EmuladorCabo::getTipoAtivo() const
{
    return _tipoAtivo;
}

int EmuladorCabo::getCanalAtivo() const
{
    return static_cast<int>(_tipoAtivo);
}

uint16_t EmuladorCabo::getResistencia() const
{
    switch (_tipoAtivo)
    {
        case TipoCabo::DESCONECTADO: return 4700;
        case TipoCabo::CABO_13A:     return 1500;
        case TipoCabo::CABO_20A:     return 680;
        case TipoCabo::CABO_32A:     return 220;
        case TipoCabo::CABO_63A:     return 100;
        case TipoCabo::FALHA_CURTO:  return 47;
        case TipoCabo::ABERTO:
        default:                     return 0; // Circuito Aberto
    }
}

int EmuladorCabo::getCorrenteMaxima() const
{
    switch (_tipoAtivo)
    {
        case TipoCabo::CABO_13A: return 13;
        case TipoCabo::CABO_20A: return 20;
        case TipoCabo::CABO_32A: return 32;
        case TipoCabo::CABO_63A: return 63;
        default:                 return 0;
    }
}

const char* EmuladorCabo::getDescricao() const
{
    switch (_tipoAtivo)
    {
        case TipoCabo::ABERTO:       return "Circuito Aberto (Sem Cabo Conectado)";
        case TipoCabo::DESCONECTADO: return "S1: Desconectado Nominal (4700R)";
        case TipoCabo::CABO_13A:     return "S2: Cabo 13 A (1500R)";
        case TipoCabo::CABO_20A:     return "S3: Cabo 20 A (680R)";
        case TipoCabo::CABO_32A:     return "S4: Cabo 32 A (220R)";
        case TipoCabo::CABO_63A:     return "S5: Cabo 63 A (100R)";
        case TipoCabo::FALHA_CURTO:  return "S6: Falha de Curto / Isolacao (47R)";
        default:                     return "Estado Desconhecido";
    }
}

void EmuladorCabo::imprimirStatus() const
{
    Serial.print(F("[STATUS CABO] Canal: "));
    Serial.print(getCanalAtivo());
    Serial.print(F(" | R: "));
    Serial.print(getResistencia());
    Serial.print(F(" Ohms | Imax: "));
    Serial.print(getCorrenteMaxima());
    Serial.print(F(" A | Descricao: "));
    Serial.println(getDescricao());
}
