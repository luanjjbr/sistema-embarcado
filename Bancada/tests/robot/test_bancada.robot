*** Settings ***
Documentation     Suite de Testes Automatizados da Bancada EV (IEC 62196-2 / IEC 61851-1)
Library           Process
Library           OperatingSystem

*** Variables ***
${PYTHON_CMD}     python
${TEST_SCRIPT}    ${CURDIR}/../python/test_cabo.py

*** Test Cases ***
Cenario 1: Execucao da Suite Completa de Testes da Bancada em Python
    [Documentation]    Executa o teste serial unitario cobrindo canais 0 a 6 e rejeicao de comandos invalidos
    ${resultado}=      Run Process    ${PYTHON_CMD}    ${TEST_SCRIPT}    stderr=STDOUT
    Log                ${resultado.stdout}
    Should Be Equal As Integers    ${resultado.rc}    0    msg=A suite de testes da bancada reportou falhas.
