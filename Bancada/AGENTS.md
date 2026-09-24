# Contexto e Diretrizes do Projeto Bancada (Sistemas Embarcados)

Este arquivo serve como contexto de inicialização e guia de regras para o assistente AI ao iniciar qualquer sessão no workspace **Bancada**.

---

## 🤝 Metodologia de Interação Obrigatória (Permanente para Todos os Chats)
- **Modo Passo a Passo com Autorização Prévia:** O assistente **NUNCA** deve aplicar alterações ou criar/modificar arquivos sem antes:
  1. Apresentar uma explicação clara e detalhada da lógica técnica (software e eletrônica) envolvida.
  2. Exibir o código exato que está sendo proposto.
  3. Solicitar expressamente a permissão/autorização do usuário.
  4. **Aguardar a confirmação explícita do usuário** antes de invocar qualquer ferramenta de modificação (`write_to_file`, `replace_file_content`, etc.).

---

## 🇧🇷 Idioma e Estilo de Comunicação
- **Obrigatório:** Comunicar-se, explicar e responder sempre em **Português do Brasil (pt-BR)**.
- Mensagens de commit, comentários em código e documentações devem estar em português, preservando termos técnicos consolidados (*baud rate*, *buffer circular*, *checksum*, *CRC-16*, *dangling pointer*, *break-before-make*, *stack overflow*, etc.).

---

## 🎯 Visão Geral do Hardware & Firmware
- **Microcontroladores Alvo:**
  - **Arduino Nano** (ATmega328P - 16 MHz, 32 KB Flash, 2 KB SRAM) -> Ambientes: `nanoatmega328` (bootloader antigo) e `nanoatmega328new` (bootloader novo).
  - **Arduino Mega 2560** (ATmega2560 - 16 MHz, 256 KB Flash, 8 KB SRAM) -> Ambiente: `megaatmega2560`.
- **Framework & SO de Tempo Real:**
  - Framework: Arduino (`platform = atmelavr`, `framework = arduino`).
  - RTOS: **FreeRTOS** (`feilipu/FreeRTOS @ ^11.1.0-3`).
- **Comunicação Serial:**
  - Baud rate padrão: **`9600 bps`** (`SERIAL_BAUD_RATE 9600`).
- **Protocolo de Comandos Serial (CLI):**
  - `cabo 0` a `cabo 6`: Ativa o canal correspondente do barramento PP e responde `OK`.
  - `cabo <invalido>`: Responde `ERRO: Cabo`.
  - `help` ou `?`: Exibe menu de comandos.
  - Qualquer outro comando: Responde `ERRO`.

---

## 📌 Mapeamento de Pinos e Recursos de Hardware (IEC 62196-2 / Pino PP)
- **Pino 13 (`PINO_LED_STATUS`):** LED onboard (*Heartbeat*), acionado pela `tarefaBlink` a cada 500 ms.
- **Pinos D4 a D9 (`PINO_CABO_S1` a `PINO_CABO_S6`):**
  - **D4 (S1):** $4700\,\Omega$ — Desconectado nominal
  - **D5 (S2):** $1500\,\Omega$ — Cabo 13 A
  - **D6 (S3):** $680\,\Omega$ — Cabo 20 A
  - **D7 (S4):** $220\,\Omega$ — Cabo 32 A
  - **D8 (S5):** $100\,\Omega$ — Cabo 63 A
  - **D9 (S6):** $47\,\Omega$ — Simulação de Falha / Curto
- **Atenção à Memória SRAM (2 KB no ATmega328P):**
  - Pilhas FreeRTOS dimensionadas estritamente em **128 words** (`STACK_BLINK 128`, `STACK_CABO 128`).
  - Nunca criar variáveis locais gigantes nas tarefas; utilize buffers estáticos ou alocação consciente para evitar colisão de heap/stack (*stack overflow*).

---

## 🏗️ Estrutura de Pastas e Componentes
- `platformio.ini`: Configuração dos 3 ambientes de compilação, monitor serial em 9600 e dependência do FreeRTOS.
- `include/ConfigHardware.h`: Mapeamento centralizado de pinos, baud rate, detecção de placa (`PLACA_NANO`, `PLACA_MEGA`) e tamanhos de stack.
- `lib/EmuladorCabo/`: Classe C++ `EmuladorCabo` (POO) com proteção *Break-Before-Make* para o pino PP.
- `src/main.cpp`: CLI serial interativa e tarefa Heartbeat com FreeRTOS.
- `docs/`: Documentações técnicas e mapa de pinos (`docs/pinout.md`, `docs/README.md`).
- `tests/`: Estrutura para automação de testes com pastas `python/`, `relatorios/` e `robot/`.
- `CONTEXTO.md`: Documentação técnica aprofundada de toda a arquitetura.

---

## ⚙️ Comandos Úteis do PlatformIO
```bash
# Compilar para Arduino Nano (Bootloader Antigo - CH340)
pio run -e nanoatmega328

# Compilar para Arduino Nano (Bootloader Novo)
pio run -e nanoatmega328new

# Compilar para Arduino Mega 2560
pio run -e megaatmega2560

# Upload
pio run -e nanoatmega328 --target upload

# Abrir Monitor Serial (9600 baud)
pio device monitor -b 9600
```
