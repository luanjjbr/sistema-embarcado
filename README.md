# 🛠️ Apoio Acadêmico — Sistemas Embarcados

Este repositório foi criado para centralizar e organizar todo o material de apoio da disciplina de **Sistemas Embarcados**. Aqui você encontrará códigos-fonte, esquemáticos, roteiros de laboratório e projetos práticos desenvolvidos ao longo do curso.

---

## 📌 Conteúdo do Repositório

* 📁 **`Aulas/`**: Códigos e projetos práticos desenvolvidos ao longo da disciplina:
  * 🔹 **`Aula1/`**: Data logger analógico com temporização não-bloqueante (`millis`) e alinhamento de memória via `union`/`struct`.
  * 🔹 **`Aula2/`**: Calibração e cálculo de $V_{CC}$ real via leitura da referência interna de 1.1V (*Bandgap*) nos registradores AVR.
  * 🔹 **`Aula3/`**: Armazenamento de parâmetros em EEPROM com validação de integridade por *checksum* e buffer circular.
  * 🔹 **`Aula4/`**: Persistência de dados com algoritmo **CRC-16 Modbus**, interface de comando UART e rotina de *dump* hexadecimal da EEPROM.
  * 🔹 **`Aula5_Malloc/`**: Alocação dinâmica de memória em C (`malloc`/`free`), tratamento de ponteiro nulo (`NULL`) e prevenção de *dangling pointers*.
  * 🔹 **`Aula6_Malloc_Integrado/`**: Sistema integrado unindo alocação dinâmica, calibração via *Bandgap*, cálculo de CRC-16 por amostra e parser serial configurável.
  * 🔹 **`Aula7_DataLogger_Final/`**: Modelo final consolidado com buffer contínuo (sem fragmentação de heap), amostragem em milivolts, integridade CRC-16 e telemetria periódica.
  * 🔹 **`Aula8/`**: Introdução a sistemas operacionais de tempo real (**FreeRTOS**), tarefas concorrentes preemptivas (`xTaskCreate`, `vTaskDelay`), leitura analógica e interface com display LCD 16x2.
  * 🔹 **`Aula9/`**: FreeRTOS multitarefa com aquisição analógica concorrente, amostragem em rajada rápida (*burst sampling* no pino 12), proteção da UART via Mutex (`xSerialMutex`) e interface com LCD 16x2.
* 📁 **[`Bancada/`](Bancada/)**: Firmware com FreeRTOS para Arduino Nano e Mega 2560 (CLI Serial 9600 baud, pinos D4-D9 e proteção Break-Before-Make).
* 📁 **[`bancada_de_teste/`](bancada_de_teste/)**: Bancada de testes de Estação de Recarga Veicular (IEC 61851-1 / IEC 62196-2) em **C++ POO**, **FreeRTOS** multitarefa (ESP32 / Arduino Nano / Mega) e homologação automatizada com **Robot Framework** e relatórios visuais.
* 📁 **`testes/`**: Testes rápidos de hardware, comunicação e validação:
  * 🔹 **`teste_serial/`**: Firmware para validação de comunicação UART (*Echo*, *Blink* concorrente e telemetria periódica).
* 📁 **`docs/`**: Literatura técnica, manuais de referência e normas:
  * 🔹 Manuais e guias de **FreeRTOS** (API Reference e Kernel Guide).
  * 🔹 Livro de referência em linguagem C (**Head First C**).
  * 🔹 **`normas_carregador_byd/`**: Normas internacionais e artigos de recarga de veículos elétricos (IEC 61851-1 Modo 3 e IEC 62196-1).

---

## 📚 Detalhamento e Explicação de Cada Aula

### 🔹 [Aula 1 — Data Logger Básico (Estruturas e Temporização)](file:///c:/Users/italo/OneDrive/Imagens/Git/sistema-embarcado/Aulas/Aula1/Aula1.ino)
* **Objetivo:** Introduzir o conceito de aquisição de dados periódica sem travar a CPU do microcontrolador.
* **Principais Conceitos:**
  * **Alinhamento e Mapeamento de Memória:** Uso conjunto de `union` e `struct` para permitir acesso aos dados analógicos tanto por nomes legíveis (`i16_Time`, `i16_adc0` a `i16_adc7`) quanto por índice indexado em loop (`i16_dados[9]`), garantindo ocupação estrita de 18 bytes.
  * **Temporização Não-Bloqueante:** Gerenciamento independente de taxas de amostragem e transmissão via `millis()`, eliminando chamadas a `delay()`.
  * **Histórico com Deslocamento:** Implementação de fila com deslocamento sequencial de amostras (`move()`).

---

### 🔹 [Aula 2 — Leitura de Registradores e Calibração de VCC](file:///c:/Users/italo/OneDrive/Imagens/Git/sistema-embarcado/Aulas/Aula2/Aula2.ino)
* **Objetivo:** Acessar diretamente os registradores de hardware do microcontrolador ATmega328P para calibrar a leitura de tensão do sistema.
* **Principais Conceitos:**
  * **Manipulação de Registradores AVR:** Configuração dos bits `REFS0`, `MUX3`, `MUX2` e `MUX1` no registrador `ADMUX` e disparo da conversão pelo `ADCSRA`.
  * **Leitura da Referência Bandgap (1.1V):** O conversor A/D mede a sua própria tensão interna fixa de referência em relação a $V_{CC}$.
  * **Cálculo Matemático da Tensão Real:**
    $$\text{VDD (mV)} = \frac{1023 \times 1100\text{ mV}}{\text{Leitura ADC}}$$
    Permite corrigir erros de medição causados por flutuações na fonte de alimentação ou bateria.

---

### 🔹 [Aula 3 — Memória EEPROM, Checksum e Buffer Circular](file:///c:/Users/italo/OneDrive/Imagens/Git/sistema-embarcado/Aulas/Aula3/Aula3.ino)
* **Objetivo:** Implementar persistência não-volátil de parâmetros de configuração e otimizar o histórico de amostras.
* **Principais Conceitos:**
  * **Persistência de Configurações:** Salva o número de canais ativos e os tempos de amostragem e UART na memória EEPROM.
  * **Validação por Checksum:** Utiliza a macro `offsetof()` para somar os bytes da estrutura e validar se a EEPROM foi corrompida ou está vazia antes de carregar valores padrão.
  * **Buffer Circular com Indexação Modular:** Substitui a função `move()` por incremento circular (`(indice + 1) % 5`), resultando em complexidade temporal $O(1)$ para inserção de amostras.
  * **Parser de Comandos:** Recepção e decodificação de comandos de texto via `sscanf()` pela UART.

---

### 🔹 [Aula 4 — Integridade com CRC-16 Modbus e Dump Hexadecimal](file:///c:/Users/italo/OneDrive/Imagens/Git/sistema-embarcado/Aulas/Aula4/Aula4.ino)
* **Objetivo:** Elevar o nível de integridade dos dados na EEPROM e implementar ferramentas de depuração em tempo de execução.
* **Principais Conceitos:**
  * **Algoritmo CRC-16 (Modbus):** Implementação da verificação cíclica de redundância com o polinômio reverso `0xA001`, padrão amplamente adotado em sistemas industriais.
  * **Comando Dump Hexadecimal:** Interface serial com comando `dump` que exibe o mapa de memória completo da EEPROM formatado em linhas de 16 bytes (`0x00: XX XX ...`).
  * **Sinalização Heartbeat:** Inversão periódica do estado do pino `LED_BUILTIN` para monitorar a saúde do loop principal.

---

### 🔹 [Aula 5 — Introdução à Alocação Dinâmica (`malloc`/`free`)](file:///c:/Users/italo/OneDrive/Imagens/Git/sistema-embarcado/Aulas/Aula5_Malloc/Aula5_Malloc.ino)
* **Objetivo:** Compreender os fundamentos do gerenciamento de memória na Heap em sistemas embarcados.
* **Principais Conceitos:**
  * **Alocação Dinâmica:** Criação de buffers em tempo de execução com `malloc(sizeof(int16_t) * tamanho)`.
  * **Tratamento de Falhas:** Checagem obrigatória do retorno contra ponteiro nulo (`NULL`), evitando acessos a endereços inválidos de memória.
  * **Desalocação e Prevenção de Falhas:** Liberação da memória com `free()` e anulação imediata do ponteiro (`ptr = NULL`) para evitar *dangling pointers* (ponteiros soltos).

---

### 🔹 [Aula 6 — Data Logger Dinâmico Integrado](file:///c:/Users/italo/OneDrive/Imagens/Git/sistema-embarcado/Aulas/Aula6_Malloc_Integrado/Aula6_Malloc_Integrado.ino)
* **Objetivo:** Construir um registrador de dados configurável combinando alocação dinâmica por amostra, conversão de unidades e proteção por CRC.
* **Principais Conceitos:**
  * **Alocação por Amostra:** Estrutura dinâmica que aloca vetores independentes para cada registro conforme a quantidade de canais selecionada.
  * **Conversão em Tempo Real:** Transforma leituras brutas de ADC (0–1023) diretamente em milivolts ($mV$) com base no $V_{CC}$ calculado continuamente via *Bandgap*.
  * **Terminal Interativo:** Parser UART capaz de reconfigurar quantidade de canais (`C<n>`), tempo de amostragem (`TA<ms>`) e tempo de envio (`TU<ms>`), liberando e realocando memória sob demanda.

---

### 🔹 [Aula 7 — Data Logger Final (Otimização Contínua de Memória e Telemetria)](file:///c:/Users/italo/OneDrive/Imagens/Git/sistema-embarcado/Aulas/Aula7_DataLogger_Final/Aula7_DataLogger_Final.ino)
* **Objetivo:** Apresentar a arquitetura definitiva e robusta para um registrador de dados em microcontroladores AVR.
* **Principais Conceitos:**
  * **Buffer Contínuo Plano (*Flat Buffer*):** Aloca todo o lote de amostras em um único bloco de memória contínuo na Heap. Elimina por completo o problema de **fragmentação de memória** inerente a múltiplos `malloc()` individuais.
  * **Dimensionamento Automático do Lote:**
    $$\text{Total de Amostras} = \frac{\text{Tempo de Envio (ex: 5000 ms)}}{\text{Tempo de Aquisição (ex: 250 ms)}} = 20\text{ amostras}$$
  * **Proteção por CRC-16 no Registro:** Cada amostra armazena os 6 canais em milivolts acrescidos do CRC-16 Modbus correspondente no último slot.
  * **Leitura Atômica AVR:** Uso do registrador nativo `ADCW` para garantir leitura segura de `ADCL` e `ADCH` sem risco de corrupção assíncrona.
  * **Relatório de Telemetria:** Saída serial formatada em tabela com verificação de integridade individual por linha (`[OK]` ou `[ERRO CRC]`) e comandos de controle (`send`, `vdd`, `reset`, `help`).

---

### 🔹 [Aula 8 — Introdução ao FreeRTOS e Display LCD](Aulas/Aula8/Aula8.ino)
* **Objetivo:** Implementar multitarefa preemptiva em microcontrolador utilizando o sistema operacional de tempo real **FreeRTOS**.
* **Principais Conceitos:**
  * **Criação e Priorização de Tarefas:** Instanciação de tasks com `xTaskCreate()` para sinalização com LED (`TaskBlink`), aquisição analógica periódica (`TaskAnalogRead`) e interface com usuário via display (`TaskDisplay`).
  * **Temporização Preemptiva:** Uso de `vTaskDelay()` para liberar a CPU durante intervalos de espera, permitindo que tarefas de menor prioridade ou mesmo nível executem sem bloqueio do processador.
  * **Concorrência com Display LCD 16x2:** Atualização contínua dos dados lidos na entrada `A0` em tela de cristal líquido de forma assíncrona e concorrente.

---

### 🔹 [Aula 9 — FreeRTOS Multitarefa, Amostragem em Rajada e Sincronização por Mutex](Aulas/Aula9/Aula9.ino)
* **Objetivo:** Explorar aquisição analógica concorrente com FreeRTOS, amostragem rápida em rajada com sinalizador em osciloscópio e proteção da UART contra colisões e estouro de buffer.
* **Principais Conceitos:**
  * **Amostragem em Rajada (*Burst Sampling*) com Pino de Trigger:** A tarefa `vNewTaskAnalogRead` realiza a leitura de 64 amostras em intervalos de 1 ms no pino A1, sinalizando o início e fim da rajada no pino digital 12 (`Ci_Pin12`) para aferição com osciloscópio ou analisador lógico.
  * **Controle de Preempção com `vTaskSuspendAll()` e `xTaskResumeAll()`:** Congelamento temporário do escalonador preemptivo durante a rajada de aquisição para garantir temporização determinística rígida.
  * **Exclusão Mútua na UART (`xSerialMutex`):** Proteção da porta Serial compartilhada entre `TaskAnalogRead` (leitura periódica de A0) e `vNewTaskAnalogRead` (amostras de A1) via `xSemaphoreCreateMutex()`, eliminando corrupção de mensagens e caracteres truncados.
  * **Formatação Eficiente de Telemetria:** Impressão das 64 amostras em formato tabular (8 amostras por linha) a 9600 bps para evitar estouro do buffer circular de 64 bytes da UART do microcontrolador.
  * **Concorrência com Display LCD 16x2:** Atualização em tempo real das variáveis globais `sensorValue` e `sensorValueA1` no display.

---

## 💻 Microcontroladores & Ferramentas Utilizadas

* **Hardware:** ESP32 (DevKit v1), Arduino Nano (ATmega328P), Arduino Mega 2560, Arduino Uno
* **Linguagens:** C / C++
* **IDEs & Ambientes:** VS Code (PlatformIO) / Arduino IDE

---

## 🚀 Como Utilizar

1. **Clone o repositório:**
   ```bash
   git clone https://github.com/luanjjbr/sistema-embarcado.git
   ```

2. **Abra o projeto:**
   * No **Arduino IDE**: Abra qualquer pasta em `Aulas/` (o nome da pasta coincide com o arquivo `.ino` correspondente).
   * No **VS Code**: Utilize a extensão PlatformIO ou Arduino para compilar e fazer upload para a placa.

---

## 🖥️ Interação com o Usuário via Serial (Terminal UART)

O firmware final ([Aula 7](file:///c:/Users/italo/OneDrive/Imagens/Git/sistema-embarcado/Aulas/Aula7_DataLogger_Final/Aula7_DataLogger_Final.ino)) conta com um terminal serial interativo que permite configurar o sistema em tempo de execução sem necessidade de recompilar ou regravar a placa.

* **Velocidade de Comunicação:** `9600 baud`
* **Configuração de Final de Linha:** `Nova Linha` (`NL` / `\n`)

---

### 📋 Tabela de Comandos Disponíveis

| Comando | Formato | O que faz | Exemplo | Resposta do Sistema |
| :--- | :--- | :--- | :---: | :--- |
| **Canais** | `C<n>` | Define o número de entradas ativas (1 a 6) | `C4` | `>> Numero de entradas alterado para: 4` |
| **Tempo UART** | `TU<ms>` | Altera o intervalo de envio serial em ms | `TU3000` | `>> Tempo de envio UART alterado para: 3000 ms` |
| **Tempo Aquisição** | `TA<ms>` | Altera o intervalo entre leituras analógicas em ms | `TA500` | `>> Tempo de aquisicao alterado para: 500 ms` |
| **Configuração 2P** | `<c>,<tu>` | Altera canais e tempo de envio juntos | `4,3000` | `>> Configuracao atualizada: 4 canais \| Envio: 3000 ms` |
| **Configuração 3P** | `<c>,<ta>,<tu>` | Altera canais, aquisição e envio de uma vez | `4,250,5000` | `>> Configuracao atualizada: 4 canais \| Aq: 250 ms \| Envio: 5000 ms` |
| **Leitura Instantânea**| `read` | Imprime a leitura instantânea de todos os canais em mV | `read` | `>> Leituras Instantaneas (mV): A0: 4980 mV \| A1: 2450 mV ...` |
| **Envio Imediato** | `send` | Força a transmissão imediata da tabela do lote | `send` | Dispara o relatório consolidado de telemetria |
| **Tensão VDD** | `vdd` | Exibe a tensão da fonte calibrada via Bandgap 1.1V | `vdd` | `>> VDD Calibrado (Alimentacao): 5012 mV` |
| **Limpeza** | `reset` | Zera todo o buffer na RAM e reinicia o lote | `reset` | `>> Buffer de telemetria reinicializado para zero.` |
| **Ajuda** | `help` | Exibe o menu com a lista de comandos | `help` | Lista todos os comandos disponíveis |

---

### 💡 Exemplos Práticos de Interação

#### Exemplo 1: Reconfigurar o Sistema para 4 Canais e Envio a cada 3 Segundos
Digite no Monitor Serial:
```text
4,500,3000
```
**Resposta do Microcontrolador:**
```text
[INFO] Buffer de memoria alocado: 60 bytes (6 amostras x 5 words). Canais: 4 | Tempo UART: 3000 ms.
>> Configuracao atualizada com sucesso: 4 entradas | Tempo Aquisicao: 500 ms | Tempo UART: 3000 ms.
```
> O microcontrolador libera a memória anterior via `free()`, calcula que $3000\text{ ms} / 500\text{ ms} = 6\text{ amostras}$ e aloca o tamanho exato na Heap com `malloc()`.

---

#### Exemplo 2: Consultar Leitura Instantânea de Todas as Entradas em Milivolts (mV)
Digite no Monitor Serial:
```text
read
```
**Resposta do Microcontrolador:**
```text
>> Leituras Instantaneas (mV): A0: 4980 mV | A1: 2450 mV | A2: 1200 mV | A3: 0 mV
```

---

#### Exemplo 3: Relatório Automático de Telemetria (Consolidado em mV com CRC-16)
A cada término de ciclo UART (ex: 5 segundos), o microcontrolador transmite automaticamente:
```text
================ RELATORIO DE TELEMETRIA (TODAS AS ENTRADAS EM mV) ================
Tensão VDD do Sistema: 5012 mV | Canais Ativos: 4 | Amostras: 6 | Intervalo UART: 3000 ms
Indice |    A0    |    A1    |    A2    |    A3    |  CRC-16  | Status
-----------------------------------------------------------------------------------
[00]   |  4980 mV |  2450 mV |  1200 mV |     0 mV | 0x1A2B   | [OK]
[01]   |  4980 mV |  2452 mV |  1198 mV |     0 mV | 0x3F81   | [OK]
[02]   |  4978 mV |  2450 mV |  1200 mV |     0 mV | 0x82C4   | [OK]
[03]   |  4980 mV |  2450 mV |  1200 mV |     0 mV | 0x1A2B   | [OK]
[04]   |  4982 mV |  2448 mV |  1202 mV |     0 mV | 0x9D4E   | [OK]
[05]   |  4980 mV |  2450 mV |  1200 mV |     0 mV | 0x1A2B   | [OK]
===================================================================================
```

---

### 🏷️ Tipos de Commit Utilizados

| Tipo | Descrição | Exemplo |
| :--- | :--- | :--- |
| **`feat`** | Nova funcionalidade ou implementação de código | `feat(Aula3): adiciona parser uart e salvar na eeprom` |
| **`fix`** | Correção de bugs, vazamentos de memória ou ponteiros | `fix(Aula6_Malloc_Integrado): corrige checagem de retorno do malloc` |
| **`docs`** | Alteração de documentação, README ou inclusão de livros | `docs: atualiza estrutura de pastas no README` |
| **`refactor`** | Reorganização do código sem alterar o comportamento | `refactor(Aula2): melhora calculo de vcc via bandgap` |
| **`style`** | Ajustes de formatação, nomes de arquivos e pastas | `style: padroniza nomes de pastas para padrao Arduino IDE` |
| **`chore`** | Atualizações de configurações do Git, `.gitignore` ou build | `chore: atualiza gitignore para ignorar arquivos temporarios` |