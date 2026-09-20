# Diretrizes Globais do Repositório - Sistemas Embarcados

Este arquivo estabelece o contexto global, regras e diretrizes de projeto do repositório **sistema-embarcado**, que centraliza materiais acadêmicos, bancadas de teste e firmware para sistemas embarcados.

---

## 🤝 Metodologia de Interação Obrigatória (Permanente para Todos os Chats)
- **Modo Passo a Passo com Autorização Prévia:** O assistente **NUNCA** deve aplicar alterações ou criar/modificar arquivos de código sem antes:
  1. Apresentar uma explicação clara e detalhada da lógica técnica (software e eletrônica) envolvida.
  2. Exibir o código exato que está sendo proposto.
  3. Solicitar expressamente a permissão/autorização do usuário.
  4. **Aguardar a confirmação explícita do usuário** antes de invocar qualquer ferramenta de modificação (`write_to_file`, `replace_file_content`, etc.).

---

## 1. Idioma e Comunicação Obrigatória
- O assistente deve se comunicar, explicar e responder sempre em **Português do Brasil (pt-BR)**.
- Explicações, comentários de código, documentações e mensagens de commit devem ser redigidos em português, preservando os termos técnicos universais (*baud rate*, *buffer circular*, *checksum*, *CRC-16*, *dangling pointer*, *break-before-make*, *stack overflow*, *task*, etc.).

---

## 2. Mapa Geral de Arquitetura do Repositório

```text
sistema-embarcado/
├── Aulas/                     # Fundamentos práticos progressivos da disciplina (Aulas 1 a 8)
│   ├── Aula1/                 # Data logger, temporização com millis(), union e struct
│   ├── Aula2/                 # Registradores AVR, calibração de VCC via Bandgap 1.1V
│   ├── Aula3/                 # EEPROM, validação por checksum e buffer circular O(1)
│   ├── Aula4/                 # Integridade por CRC-16 Modbus e comando dump hexadecimal
│   ├── Aula5_Malloc/          # Alocação dinâmica de memória (malloc/free) e proteção a ponteiros nulos
│   ├── Aula6_Malloc_Integrado/# Data logger dinâmico com parser serial interativo
│   ├── Aula7_DataLogger_Final/# Flat Buffer contínuo (zero fragmentação) e telemetria formatada
│   └── Aula8/                 # Introdução ao FreeRTOS (multitarefa preemptiva) e display LCD
├── Bancada/                   # Workspace Ativo: Firmware com FreeRTOS para Arduino Nano e Mega
│   ├── platformio.ini         # Ambientes: nanoatmega328, nanoatmega328new, megaatmega2560 (9600 baud)
│   ├── include/ConfigHardware.h # Pinos D13 (LED) e D4 a D9 (Cabo S1-S6), stacks de 128 words
│   ├── lib/EmuladorCabo/      # Classe C++ POO para emulação do sinal do cabo com Break-Before-Make
│   ├── src/main.cpp           # CLI serial interativa e tarefas preemptivas FreeRTOS
│   ├── docs/                  # Documentação e pinout da bancada
│   └── tests/                 # Estrutura para automação de testes (Python, Robot Framework)
├── bancada_de_teste/          # Projeto Avançado: Estação de Recarga Veicular (IEC 61851-1 / IEC 62196-2)
│   ├── platformio.ini         # Suporte a ESP32 e Arduino Nano / Mega (115200 baud)
│   ├── include/ConfigHardware.h # Barramento de resistores do pino PP (chaves MOSFET S1 a S6)
│   ├── lib/EmuladorCabo/      # Emulador do pino PP com intertravamento break-before-make
│   ├── src/main.cpp           # CLI serial interativa e tarefas FreeRTOS
│   └── tests/                 # Testes de bancada automatizados com Robot Framework e Python
├── docs/                      # Manuais técnicos do FreeRTOS, livro Head First C e normas IEC
└── testes/                    # Testes rápidos de hardware e comunicação (ex: teste_serial)
```

---

## 3. Boas Práticas do Repositório
- **Código Limpo e Modular:** Separar cabeçalhos (`include/`), bibliotecas de domínio (`lib/`) e código de aplicação (`src/`).
- **Gerenciamento de Tempo:** Priorizar temporização não-bloqueante (`millis`) em sistemas cooperativos e `vTaskDelay` em sistemas com FreeRTOS. **Evitar totalmente `delay()` bloqueante**.
- **Gestão de Memória em AVR:** Atenção estrita ao limite de SRAM (2 KB no ATmega328P). Manter stacks de tarefas em 128 words, evitar alocações dinâmicas contínuas na heap que causem fragmentação (utilizar padrão *Flat Buffer* conforme Aula 7).
- **Tratamento de Registradores:** Manipulação direta de registradores AVR (`ADMUX`, `ADCSRA`, `ADCW`) deve ser claramente documentada e comentada.
- **Padrão de Commits:** Utilizar *Conventional Commits* (`feat:`, `fix:`, `docs:`, `refactor:`, `style:`, `chore:`).
