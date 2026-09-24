# 🤖 Diretrizes e Manifesto do Agente — Sistemas Embarcados

Este arquivo estabelece o contexto global, regras operacionais e diretrizes arquiteturais do repositório **sistema-embarcado**, que centraliza materiais práticos acadêmicos, bancadas de teste de hardware e firmwares em C/C++ com FreeRTOS.

---

## 🤝 Metodologia de Interação Obrigatória (Permanente para Todos os Chats)

O assistente e os agentes operando neste repositório devem seguir rigorosamente o **Modo Passo a Passo com Autorização Prévia (Human-in-the-Loop)**:

1. **Explicação Técnica Prévia:** Apresentar uma explicação clara e detalhada da lógica técnica (software embarcado e eletrônica/hardware) envolvida.
2. **Exibição do Código Proposto:** Exibir o trecho exato ou arquivo completo que está sendo proposto antes de gravá-lo.
3. **Solicitação Expressa de Autorização:** Solicitar explicitamente a permissão do usuário para realizar a alteração.
4. **🛑 Parada Obrigatória (Turno Separado):**
   > [!IMPORTANT]
   > É **TERMINANTEMENTE PROIBIDO** invocar ferramentas de escrita (`write_to_file`, `replace_file_content`, etc.) no mesmo turno em que a proposta é apresentada. O agente deve finalizar sua resposta e aguardar uma nova mensagem do usuário autorizando expressamente a alteração (ex: *"Pode aplicar"*, *"Aprovado"*, *"Prossiga"*).

---

## 🇧🇷 1. Idioma e Comunicação Obrigatória
- **Comunicação estritamente em Português do Brasil (pt-BR):** Respostas, explicações, documentações, decisões e mensagens de commit devem ser sempre em português brasileiro.
- **Jargão Técnico Universal:** Preservar a nomenclatura técnica consolidada (*baud rate*, *buffer circular*, *checksum*, *CRC-16*, *dangling pointer*, *break-before-make*, *stack overflow*, *task*, *heap*, etc.).

---

## 🌳 2. Mapa Arquitetural do Repositório

```text
sistema-embarcado/
├── Aulas/                         # Fundamentos práticos progressivos da disciplina (Aulas 1 a 9)
│   ├── Aula1/                     # Data logger, temporização com millis(), union e struct
│   ├── Aula2/                     # Registradores AVR, calibração de VCC via Bandgap 1.1V
│   ├── Aula3/                     # EEPROM, validação por checksum e buffer circular O(1)
│   ├── Aula4/                     # Integridade por CRC-16 Modbus e comando dump hexadecimal
│   ├── Aula5_Malloc/              # Alocação dinâmica de memória (malloc/free) e proteção a ponteiros nulos
│   ├── Aula6_Malloc_Integrado/    # Data logger dinâmico com parser serial interativo
│   ├── Aula7_DataLogger_Final/    # Flat Buffer contínuo (zero fragmentação) e telemetria formatada
│   ├── Aula8/                     # Introdução ao FreeRTOS (multitarefa preemptiva) e display LCD
│   └── Aula9/                     # FreeRTOS multitarefa, leitura analógica concorrente e LiquidCrystal
│
├── Bancada/                       # Workspace Ativo: Firmware com FreeRTOS para Arduino Nano e Mega
│   ├── platformio.ini             # Ambientes: nanoatmega328, nanoatmega328new, megaatmega2560 (9600 baud)
│   ├── include/ConfigHardware.h   # Pinos D13 (LED) e D4 a D9 (Cabo S1-S6), stacks de 128 words
│   ├── lib/EmuladorCabo/          # Classe C++ POO para emulação do pino PP com Break-Before-Make
│   ├── src/main.cpp               # CLI serial interativa e tarefas preemptivas FreeRTOS
│   ├── docs/                      # Documentação e pinout da bancada
│   └── tests/                     # Estrutura para automação de testes (Python, Robot Framework)
│
├── bancada_de_teste/              # Projeto Avançado: Estação de Recarga Veicular (IEC 61851-1 / IEC 62196-2)
│   ├── platformio.ini             # Ambientes para ESP32 e Arduino Nano / Mega (115200 baud)
│   ├── include/ConfigHardware.h   # Barramento de resistores do pino PP (chaves MOSFET S1 a S6)
│   ├── lib/EmuladorCabo/          # Emulador do pino PP com intertravamento break-before-make
│   ├── src/main.cpp               # CLI serial interativa e tarefas FreeRTOS
│   └── tests/                     # Testes automatizados com Robot Framework e Python
│
├── knowledge/                     # 📚 Base de Conhecimento Modularizada
│   ├── 01_governanca.md           # Metodologia Human-in-the-Loop, turnos e commits
│   ├── 02_hardware_avr.md         # Registradores AVR, Bandgap 1.1V e limite de 2KB SRAM
│   ├── 03_freertos_guidelines.md  # Dimensionamento de Stacks (128 words), vTaskDelay e filas
│   └── domains/                   # 🎯 Regras Técnicas por Domínio Funcional
│       ├── emulador_pp_iec62196.md# Norma IEC 62196-2, tabela de resistores e Break-Before-Make
│       ├── protocolo_serial_cli.md# Especificação do parser serial CLI (cabo 0..6, OK, ERRO)
│       └── aulas_academicas.md    # Resumo conceitual dos módulos práticos (Aulas 1 a 9)
│
├── audit/                         # 📋 Relatórios de Auditoria e Testes de Bancada
│   └── .gitkeep                   # Rastreamento do diretório no repositório
│
├── docs/                          # Manuais técnicos do FreeRTOS, livro Head First C e normas IEC
└── testes/                        # Testes rápidos de hardware e comunicação (ex: teste_serial)
```

---

## 📖 3. Módulos de Conhecimento (`knowledge/`)

Ao trabalhar em demandas específicas, consulte o módulo correspondente:

| Módulo | Arquivo | Finalidade e Conteúdo |
| :--- | :--- | :--- |
| **Governança & Git** | [`knowledge/01_governanca.md`](knowledge/01_governanca.md) | Parada obrigatória em turno separado, fluxo de trabalho e commits padronizados. |
| **Hardware AVR** | [`knowledge/02_hardware_avr.md`](knowledge/02_hardware_avr.md) | Registradores de baixo nível, Bandgap 1.1V e restrições de SRAM (2 KB). |
| **FreeRTOS** | [`knowledge/03_freertos_guidelines.md`](knowledge/03_freertos_guidelines.md) | Stacks de 128 words, temporização não-bloqueante (`vTaskDelay`) e tarefas preemptivas. |

---

## 🎯 4. Domínios de Aplicação (`knowledge/domains/`)

Regras técnicas e peculiaridades funcionais específicas do hardware e do firmware:

| Domínio | Arquivo de Referência | Resumo Técnico |
| :--- | :--- | :--- |
| **Pino PP (IEC 62196-2)** | [`knowledge/domains/emulador_pp_iec62196.md`](knowledge/domains/emulador_pp_iec62196.md) | Chaves S1 a S6, tabela de resistores (4700R a 47R) e intertravamento Break-Before-Make. |
| **Protocolo Serial CLI** | [`knowledge/domains/protocolo_serial_cli.md`](knowledge/domains/protocolo_serial_cli.md) | Comandos `cabo 0..6`, respostas `OK`/`ERRO`, baud rate 9600 / 115200 bps. |
| **Aulas Práticas** | [`knowledge/domains/aulas_academicas.md`](knowledge/domains/aulas_academicas.md) | Referência e conceitos fundamentais das Aulas 1 a 9. |

---

## ⚡ 5. Boas Práticas e Diretrizes de Firmware Embarcado

### 5.1. Gestão Rigorosa de Memória (SRAM de 2 KB no ATmega328P)
- **Tamanho de Stacks no FreeRTOS:** Cada tarefa deve ter sua pilha dimensionada com parcimônia (padrão de **128 words** / 256 bytes em `STACK_BLINK` e `STACK_CABO`).
- **Prevenção de Stack Overflow:** Proibido instanciar buffers grandes ou estruturas pesadas no escopo local de tarefas.
- **Fragmentação de Heap:** Evitar chamadas repetitivas de `malloc`/`free` durante a execução normal. Dar preferência a buffers estáticos ou alocação contínua (*Flat Buffer*, conforme padrão da Aula 7).

### 5.2. Temporização e Preempção
- **Proibição de `delay()` Bloqueante:** O uso de `delay()` bloqueante é expressamente vedado em firmwares com RTOS.
- Utilizar `vTaskDelay()` ou `vTaskDelayUntil()` para tarefas do FreeRTOS.
- Em códigos bare-metal/cooperativos (Aulas iniciais), utilizar temporização não-bloqueante baseada em `millis()`.

### 5.3. Hardware e Registradores de Baixo Nível
- **Documentação de Registradores:** Manipulações diretas nos registradores AVR (`ADMUX`, `ADCSRA`, `ADCW`, `DDRx`, `PORTx`, `PINx`) devem conter comentários explicando o propósito de cada bit configurado.
- **Intertravamento Elétrico (Break-Before-Make):** Na comutação de resistores de barramento (como na emulação do pino PP da norma IEC 62196-2), sempre desacionar a chave anterior antes de acionar a nova, evitando curto-circuito ou leituras de resistências espúrias em paralelo.

---

## 🛠️ 6. Padrões de Projeto e Versionamento
- **Modularização de Código:** Separar rigorosamente cabeçalhos (`include/`), bibliotecas de domínio/drivers (`lib/`) e código principal da aplicação (`src/`).
- **Padrão de Commits:** Utilizar *Conventional Commits* em mensagens no Git (`feat:`, `fix:`, `docs:`, `refactor:`, `chore:`).
