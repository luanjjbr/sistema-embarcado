*** Settings ***
Documentation     Suite de Testes da Bancada - Emulação e Validação de Protocolo
...               Suporta múltiplos cenários e passos com nomes descritivos.
Library           EmuladorCaboLib.py
Suite Setup       Conectar Emulador    ${PORTA}    ${BAUDRATE}
Suite Teardown    Desconectar Emulador

*** Variables ***
${PORTA}          COM5       # Altere para COM3 se estiver usando o ESP32
${BAUDRATE}       115200

*** Keywords ***
Executar Passo
    [Arguments]    ${nome_passo}    ${comando}    ${esperado}
    ${info}=    Testar Passo    ${nome_passo}    ${comando}    ${esperado}
    Set Test Message    ${info}

*** Test Cases ***
Cenario 1: Emulacao de Cabos de Recarga (Pino PP)
    [Documentation]    Comuta e valida todos os resistores normativos de capacidade do cabo (IEC 62196-2).
    Executar Passo    Ativar Cabo Desconectado (S1 - 4700R)    cabo 1    OK
    Executar Passo    Ativar Cabo de 13A (S2 - 1500R)          cabo 2    OK
    Executar Passo    Ativar Cabo de 20A (S3 - 680R)           cabo 3    OK
    Executar Passo    Ativar Cabo de 32A (S4 - 220R)           cabo 4    OK
    Executar Passo    Ativar Cabo de 63A (S5 - 100R)           cabo 5    OK
    Executar Passo    Simular Falha no Cabo (S6 - 47R)         cabo 6    OK
    Executar Passo    Desligar Cabo (Circuito Aberto)          cabo 0    OK

Cenario 2: Validacao de Seguranca e Tratamento de Erros
    [Documentation]    Garante que comandos inexistentes e parâmetros fora de faixa sejam rejeitados.
    Executar Passo    Rejeitar Cabo Inexistente (99)           cabo 99             ERRO
    Executar Passo    Rejeitar Comando Desconhecido            comando_invalido    ERRO
