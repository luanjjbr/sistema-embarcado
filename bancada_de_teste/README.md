# 🎛️ Bancada de Teste Multi-Plataforma (PlatformIO)

Projeto unificado para testes e validação de bancada em sistemas embarcados, compatível com três famílias de microcontroladores utilizando o mesmo código-fonte C/C++:
* 🔹 **ESP32** (DevKit v1 / 32 bits Xtensa Dual-Core)
* 🔹 **Arduino Nano** (ATmega328P / 8 bits AVR)
* 🔹 **Arduino Mega 2560** (ATmega2560 / 8 bits AVR)

---

## 🚀 Como Funciona o Projeto

O arquivo [`platformio.ini`](platformio.ini) define **múltiplos ambientes (*environments*)**. O código em [`src/main.cpp`](src/main.cpp) utiliza as definições de hardware em [`include/config_hardware.h`](include/config_hardware.h) para configurar dinamicamente:
* Pinos de LED de status e botão de teste.
* Resolução do conversor A/D (12 bits para ESP32 e 10 bits para Nano/Mega).
* Tensão de operação para cálculo em milivolts ($3.3\text{ V}$ para ESP32 e $5.0\text{ V}$ para AVR).
* Funções de diagnóstico de clock e memória livre.

---

## 📌 Ambientes Configurados

| Ambiente no PlatformIO | Microcontrolador | Tensão | ADC | LED Status | Botão Teste |
| :--- | :--- | :--- | :--- | :--- | :--- |
| `[env:esp32dev]` | ESP32 (WROOM-32) | 3.3V | 12 bits (0-4095) | GPIO 2 | GPIO 0 (BOOT) |
| `[env:nanoatmega328]` | Arduino Nano (Bootloader antigo) | 5.0V | 10 bits (0-1023) | Pino 13 | Pino D2 |
| `[env:nanoatmega328new]` | Arduino Nano (Bootloader novo) | 5.0V | 10 bits (0-1023) | Pino 13 | Pino D2 |
| `[env:megaatmega2560]` | Arduino Mega 2560 | 5.0V | 10 bits (0-1023) | Pino 13 | Pino D2 |

---

## 💻 Como Gravar no Microcontrolador

### Opção 1: Pela Interface Gráfica do VS Code

1. Abra a pasta `bancada_de_teste` no VS Code com a extensão **PlatformIO IDE** instalada.
2. Na **Barra de Status Inferior** do VS Code:
   * Clique no seletor de ambiente (ex: `env:esp32dev` ou `Default`).
   * Escolha qual placa você conectou via USB:
     * `env:esp32dev` para ESP32
     * `env:nanoatmega328` para Arduino Nano
     * `env:megaatmega2560` para Arduino Mega 2560
3. Clique no botão de **Check (Build)** (`✔`) para compilar.
4. Clique na **Seta (Upload)** (`➜`) para gravar o firmware na placa conectada.
5. Clique no ícone de **Tomada (Serial Monitor)** para abrir o monitor serial (já configurado a **115200 baud**).

---

### Opção 2: Pelo Terminal (PlatformIO Core / CLI)

Dentro do diretório `bancada_de_teste/`, execute:

#### Para compilar e gravar no ESP32:
```bash
pio run -e esp32dev -t upload
```

#### Para compilar e gravar no Arduino Nano:
```bash
# Bootloader clássico (clones com CH340):
pio run -e nanoatmega328 -t upload

# Bootloader recente:
pio run -e nanoatmega328new -t upload
```

#### Para compilar e gravar no Arduino Mega 2560:
```bash
pio run -e megaatmega2560 -t upload
```

#### Para abrir o Monitor Serial:
```bash
pio device monitor -b 115200
```

---

## 📟 Comandos do Terminal Serial (CLI Interativo)

Ao abrir o monitor serial em **115200 baud**, você verá o banner de identificação de hardware. Você pode enviar os seguintes comandos:

* `help` ou `?`: Lista todos os comandos disponíveis.
* `info`: Exibe dados de telemetria, clock, memória livre e tempo de funcionamento (*uptime*).
* `adc`: Realiza a leitura instantânea dos canais analógicos e converte para milivolts ($mV$).
* `led on`: Força o LED da bancada a ficar continuamente ligado.
* `led off`: Apaga o LED da bancada.
* `led blink`: Retorna o LED para o modo pisca-pisca não-bloqueante (*Heartbeat* a cada 500 ms).
* `echo <mensagem>`: Devolve a mensagem enviada (teste de transmissão UART).
* `reset`: Reinicia o microcontrolador via software.
