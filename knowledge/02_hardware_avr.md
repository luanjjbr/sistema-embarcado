# ⚡ Diretrizes de Hardware e Registradores AVR (ATmega328P / ATmega2560)

## 1. Restrições Críticas de Memória
- **ATmega328P (Arduino Nano/Uno):** 32 KB Flash, **2 KB SRAM**, 1 KB EEPROM.
- **ATmega2560 (Arduino Mega):** 256 KB Flash, **8 KB SRAM**, 4 KB EEPROM.
- **Prevenção de Stack Overflow:** Proibido alocar grandes matrizes ou structs em escopo local dentro de tarefas FreeRTOS.
- **Fragmentação da Heap:** Evitar chamadas consecutivas de `malloc`/`free`. Priorizar memória estática ou *Flat Buffer* contínuo (padrão Aula 7).

## 2. Manipulação de Registradores de Baixo Nível
- Registradores de ADC (`ADMUX`, `ADCSRA`, `ADCW`) e GPIO (`DDRx`, `PORTx`, `PINx`) devem ser manipulados com operações bitwise documentadas:
  - Exemplo: Leitura de tensão de alimentação interna via Bandgap 1.1V (Aula 2).
  - Sempre incluir comentários detalhando o papel de cada bit ativado/desativado.
