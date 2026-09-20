# 🎛️ Bancada de Testes de Estação de Recarga Veicular (IEC 61851-1 / IEC 62196-2)

Sistema modular para emulação de cabos e testes automatizados de Estações de Recarga de Veículos Elétricos (**EVSE - Modo 3**), desenvolvido em **C++ Orientado a Objetos (POO)** com **FreeRTOS** multitarefa e suíte de homologação automatizada no **Robot Framework**.

Baseado nas diretrizes e circuitos do artigo técnico:  
*Desenvolvimento de uma Estação de Recarga Modo 3 para Veículos Elétricos* (Rosa et al., Universidade Federal de Santa Maria - UFSM).

---

## 📌 O que Foi Desenvolvido

### 1. Firmware Modular com POO e FreeRTOS
* **Orientação a Objetos ([`lib/EmuladorCabo/`](lib/EmuladorCabo/)):** Classe `EmuladorCabo` com métodos encapsulados (`configurarPinos()`, `desligarTodas()`, `ativarChave()`, `ativarPorCabo()`).
* **Intertravamento *Break-Before-Make*:** Todas as chaves são abertas antes de acionar o próximo resistor, prevenindo curto-circuito ou associação em paralelo de resistores no pino PP.
* **Multitarefa Preemptiva com FreeRTOS:**
  * `tarefaHeartbeat` (Prioridade 1): Pisca o LED indicador a cada 500 ms via `vTaskDelay()`, sinalizando a integridade e saúde da CPU.
  * `tarefaSerial` (Prioridade 2): Processa os comandos de texto recebidos na UART assincronamente e aciona o emulador sem bloquear o sistema.
* **Compatibilidade Multi-Alvo:** Compilação transparente para **Arduino Nano (ATmega328P)** com alocação enxuta de memória, **ESP32 DevKit** e **Arduino Mega 2560**.

### 2. Protocolo de Comunicação Serial Padronizado
* **Comandos Válidos:** Resposta positiva padronizada informando chave, resistor e capacidade:
  ```text
  OK: CABO <N> (<Descrição da Norma>)
  ```
* **Tratamento de Erros e Proteção:** Rejeição explícita com `ERRO: ...` e orientação para o comando `help` em caso de comando desconhecido ou índice inexistente.

### 3. Suíte de Testes Automatizados no Robot Framework
* **Cenários Estruturados:**
  * `Cenario 1: Emulacao de Cabos de Recarga (Pino PP)`: Comutação sequencial de todas as resistências normativas (S1 a S6 e Aberto).
  * `Cenario 2: Validacao de Seguranca e Tratamento de Erros`: Testes negativos verificando a rejeição de comandos inválidos e parâmetros fora de faixa.
* **Passos com Nomes Descritivos:** Cada etapa possui identificação clara da ação executada (ex: `Ativar Cabo de 13A (S2 - 1500R)`).
* **Integração com VS Code:** Descoberta e execução direta com um clique pelo painel de testes (**RobotCode**).

### 4. Geração Automática de Relatório Limpo e Simplificado
* **Formatos:** Exportação automática em **HTML** (`tests/relatorios/relatorio_simplificado.html`) e **Markdown** (`tests/relatorios/relatorio_simplificado.md`).
* **Estrutura por Cenário:**
  | # | Nome do Passo | Horário | Comando Enviado | Resposta Recebida | Tempo | Status |
  | :-: | :--- | :-: | :--- | :--- | :-: | :-: |
  | 1 | **Ativar Cabo Desconectado (S1 - 4700R)** | 21:43:04 | `cabo 1` | `OK: CABO 1 (...)` | 626 ms | ✅ PASSOU |

---

## 📁 Regras de Pastas e Arquitetura do Repositório

O projeto segue rigorosamente o padrão de arquitetura modular do **PlatformIO**:

```
bancada_de_teste/
│
├── docs/                         # Documentação técnica e esquemas da bancada
│   └── pinout.md                 # Mapeamento completo de pinagem, tensões e resistores
│
├── include/                      # Cabeçalhos globais e configurações de hardware
│   └── ConfigHardware.h          # Mapeamento estático de pinos (Nano e ESP32)
│
├── lib/                          # Módulos POO independentes (PlatformIO LDF)
│   ├── EmuladorCabo/             # Subsistema 1: Emulador do Cabo (Pino PP)
│   │   ├── EmuladorCabo.h        # Declaração da classe
│   │   └── EmuladorCabo.cpp      # Implementação e Break-Before-Make
│   └── EmuladorVeiculo/          # Subsistema 2: Estados A, B, C, D (Pino CP) [Futuro]
│
├── src/                          # Ponto de entrada da aplicação
│   └── main.cpp                  # Inicialização, criação das tarefas FreeRTOS e parser CLI
│
├── tests/                        # Automação de testes e qualidade de software
│   ├── python/                   # Validação autônoma de ciclo único
│   │   └── robot_teste_cabo.py
│   ├── robot/                    # Suíte formal do Robot Framework
│   │   ├── teste_emulador.robot  # Cenários e casos de teste descritivos
│   │   └── EmuladorCaboLib.py    # Biblioteca de comunicação serial e gerador de relatórios
│   └── relatorios/               # Histórico e artefatos de saída dos testes
│       ├── relatorio_simplificado.html # Relatório visual para navegadores
│       └── relatorio_simplificado.md   # Relatório consolidado em Markdown
│
├── platformio.ini                # Definição multi-ambiente de compilação
├── requirements.txt              # Dependências Python (Robot Framework e pyserial)
└── README.md                     # Manual principal do projeto
```

### 📋 Regras e Diretrizes para Cada Pasta:

1. **`lib/<NomeDoModulo>/` (Módulos POO):**
   * Cada subsistema de hardware ou emulação de norma deve residir em sua própria subpasta dentro de `lib/`.
   * Deve conter obrigatoriamente um arquivo de cabeçalho (`.h`) com a interface pública da classe e a implementação (`.cpp`).
   * **Regra:** O código do módulo **não deve** depender do `src/main.cpp`. Os pinos e parâmetros devem ser injetados via construtor ou método de inicialização.

2. **`include/` (Configurações Globais):**
   * Destinada apenas a constantes globais, macros de compilação e mapeamento de hardware (`ConfigHardware.h`).
   * **Regra:** Nenhuma lógica executável ou instância de objeto deve ser declarada no `include/`.

3. **`src/` (Aplicação Principal):**
   * Contém apenas o `main.cpp`. É o integrador do sistema: instancia os módulos de `lib/`, define a pinagem através de `include/ConfigHardware.h` e inicializa o escalonador do FreeRTOS.
   * **Regra:** O `main.cpp` deve ser mantido enxuto, delegando todo o controle de atuadores e chaves para as classes em `lib/`.

4. **`docs/` (Literatura e Pinagem):**
   * Documentação dos circuitos físicos, tabelas de equivalência de pinos, cálculos de resistores e notas das normas IEC.

5. **`tests/` (Automação de Testes):**
   * `tests/robot/`: Casos de teste declarativos legíveis em `.robot` e bibliotecas em Python.
   * `tests/python/`: Scripts rápidos e autônomos para validação direta em linha de comando.
   * `tests/relatorios/`: Destino de todos os artefatos de homologação gerados. Arquivos transitórios de log são filtrados pelo `.gitignore`.

---

## ⚡ Tabela Normativa de Canais do Cabo (Pino PP)

| Chave | Resistor ($R_{PP}$) | Capacidade de Corrente | Pino Nano | Pino ESP32 | Comando UART | Resposta Padronizada |
| :---: | :---: | :---: | :---: | :---: | :---: | :--- |
| **0** | Aberto ($\infty$) | 0 A | Nenhum (LOW) | Nenhum (LOW) | `cabo 0` | `OK: CABO 0 (PADRAO (Aberto Total...))` |
| **S1** | $4700\,\Omega$ | 0 A (Desconectado) | **D4** | **GPIO 16** | `cabo 1` | `OK: CABO 1 (S1 ATIVA: 4700 Ohms...)` |
| **S2** | $1500\,\Omega$ | **13 A** | **D5** | **GPIO 17** | `cabo 2` | `OK: CABO 2 (S2 ATIVA: 1500 Ohms...)` |
| **S3** | $680\,\Omega$ | **20 A** | **D6** | **GPIO 18** | `cabo 3` | `OK: CABO 3 (S3 ATIVA:  680 Ohms...)` |
| **S4** | $220\,\Omega$ | **32 A** | **D7** | **GPIO 19** | `cabo 4` | `OK: CABO 4 (S4 ATIVA:  220 Ohms...)` |
| **S5** | $100\,\Omega$ | **63 A / 70 A** | **D8** | **GPIO 21** | `cabo 5` | `OK: CABO 5 (S5 ATIVA:  100 Ohms...)` |
| **S6** | $47\,\Omega$ | 0 A (Falha/Curto) | **D9** | **GPIO 22** | `cabo 6` | `OK: CABO 6 (S6 ATIVA:   47 Ohms...)` |

---

## 🚀 Como Rodar Tudo (Passo a Passo)

### 1. Pré-Requisitos
* [Visual Studio Code](https://code.visualstudio.com/) com extensões:
  * **PlatformIO IDE**
  * **RobotCode** (`d-biehl.robotcode`)
* Python 3.10 ou superior instalado.

### 2. Configurar o Ambiente Python de Testes
No terminal do projeto (`bancada_de_teste/`), crie e ative o ambiente virtual:

```powershell
# Criação do ambiente virtual
python -m venv .venv

# Ativação no Windows (PowerShell)
.\.venv\Scripts\Activate.ps1

# Instalação das dependências
pip install -r requirements.txt
```

---

### 3. Compilar e Gravar o Firmware

Conecte a placa via USB e utilize o PlatformIO:

* **Gravar no Arduino Nano (ex: porta `COM5`):**
  ```powershell
  pio run -e nanoatmega328new -t upload
  ```
* **Gravar no ESP32 DevKit (ex: porta `COM3`):**
  ```powershell
  pio run -e esp32dev -t upload
  ```
* **Gravar no Arduino Mega 2560:**
  ```powershell
  pio run -e megaatmega2560 -t upload
  ```

* **Abrir Monitor Serial Interativo (115200 bps):**
  ```powershell
  pio device monitor -b 115200
  ```

---

### 4. Executar os Testes Automatizados

#### Opção A: Pelo Painel Gráfico de Testes do VS Code (Recomendado)
1. Certifique-se de que a extensão **RobotCode** está instalada.
2. Abra a barra lateral de **Testes (ícone de tubo de ensaio 🧪)** no VS Code.
3. Localize a suíte `Teste Emulador` e clique no botão **Play (`▶`)** para rodar todos os cenários ou selecione um cenário individual.
4. Os testes serão executados e todos os passos passarão para verde (`✔ PASS`).

#### Opção B: Pelo Terminal com o Robot Framework
```powershell
.\.venv\Scripts\python.exe -m robot -d tests/relatorios tests/robot/teste_emulador.robot
```

#### Opção C: Validação Rápida Autônoma em Python
```powershell
.\.venv\Scripts\python.exe tests/python/robot_teste_cabo.py
```
*(Suporta parâmetros como `--port COM5` e `--interval 0.5`)*

---

### 5. Visualizar os Relatórios Gerados

Ao término da suíte de testes, os relatórios são gerados na pasta `tests/relatorios/`:

* **Visualizar o Relatório Visual no Navegador:**
  ```powershell
  Start-Process .\tests\relatorios\relatorio_simplificado.html
  ```
* **Visualizar o Relatório em Markdown:**
  Abra diretamente no VS Code o arquivo [`tests/relatorios/relatorio_simplificado.md`](tests/relatorios/relatorio_simplificado.md).
