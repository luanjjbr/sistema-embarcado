# Bancada

Projeto de bancada baseado em **PlatformIO** com sistema operacional de tempo real **FreeRTOS** para microcontroladores AVR.

---

## 🎯 Especificações do Projeto

- **Microcontroladores Suportados:**
  - **Arduino Nano** (ATmega328P - Bootloader Antigo e Novo)
  - **Arduino Mega 2560** (ATmega2560)
- **Comunicação Serial:** `9600 bps` (Baud Rate)
- **Framework:** Arduino com `feilipu/FreeRTOS`
- **Exemplo Base:** Multitarefa *Blink* com temporização não-bloqueante (`vTaskDelay`).

---

## 📂 Estrutura de Pastas

A estrutura segue rigorosamente o padrão do projeto `bancada_de_teste`:

```text
Bancada/
├── .gitignore             # Regras de exclusão do Git
├── platformio.ini         # Configuração dos ambientes de compilação (Nano e Mega)
├── README.md              # Documentação principal do projeto
├── docs/                  # Documentações técnicas e especificações
│   └── README.md
├── include/               # Arquivos de cabeçalho (.h)
│   └── ConfigHardware.h  # Definições de hardware e pinos
├── lib/                   # Bibliotecas e módulos reutilizáveis (POO)
├── src/                   # Código-fonte principal
│   └── main.cpp           # Aplicação com FreeRTOS e Blink
├── tests/                 # Testes automatizados
│   ├── python/            # Scripts de automação em Python
│   ├── relatorios/        # Relatórios de testes gerados
│   └── robot/             # Cenários de teste do Robot Framework
└── .vscode/               # Configurações do VS Code
    └── settings.json
```

---

## 🚀 Como Compilar e Enviar

### 1. Arduino Nano (Bootloader Antigo - Clones CH340)
```bash
pio run -e nanoatmega328 --target upload
```

### 2. Arduino Nano (Bootloader Novo)
```bash
pio run -e nanoatmega328new --target upload
```

### 3. Arduino Mega 2560
```bash
pio run -e megaatmega2560 --target upload
```

### 4. Monitor Serial
Para abrir o monitor serial a 9600 baud:
```bash
pio device monitor -b 9600
```

