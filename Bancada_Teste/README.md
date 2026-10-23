# Bancada de Testes de Cabos EV — Arduino IDE

Este diretório contém a versão do firmware da **Bancada de Testes (IEC 62196-2 / IEC 61851-1)** preparada especificamente para compilação e gravação direta através da **Arduino IDE**.

---

## 🛠️ Requisitos de Software

1. **Arduino IDE** (Versão 1.8.x ou 2.x);
2. **Biblioteca FreeRTOS:**
   - Na Arduino IDE, abra o menu: `Sketch` -> `Incluir Biblioteca` -> `Gerenciar Bibliotecas...` (ou `Ctrl + Shift + I`);
   - Busque por: `FreeRTOS` (autor: **Richard Barry / Phillip Stevens (feilipu)**);
   - Instale a biblioteca `FreeRTOS`.

---

## 🔌 Pinagem do Barramento PP (IEC 62196-2)

| Sinal / Chave | Pino Arduino (Nano / Uno / Mega) | Resistor Nominal ($R_{PP}$) | Capacidade de Corrente | Função Normativa |
| :---: | :---: | :---: | :---: | :--- |
| **STATUS** | **D13** | — | — | LED Onboard (*Heartbeat* FreeRTOS a cada 500 ms) |
| **0** | — | $\infty$ (Aberto) | 0 A | Todas as chaves desligadas (Sem cabo) |
| **S1** | **D4** | $4700\,\Omega$ | 0 A | Desconectado nominal (cabo solto) |
| **S2** | **D5** | $1500\,\Omega$ | **13 A** | Cabo monofásico 13 A |
| **S3** | **D6** | $680\,\Omega$ | **20 A** | Cabo 20 A |
| **S4** | **D7** | $220\,\Omega$ | **32 A** | Cabo 32 A (comercial mais comum) |
| **S5** | **D8** | $100\,\Omega$ | **63 A** | Cabo trifásico de alta potência (63 A / 70 A) |
| **S6** | **D9** | $47\,\Omega$ | 0 A (Falha) | Simulação de falha de isolação / curto |

> [!IMPORTANT]
> **Intertravamento *Break-Before-Make*:**  
> O firmware sempre desliga fisicamente todas as chaves MOSFET antes de ligar a nova chave selecionada, evitando resistências espúrias em paralelo no barramento.

---

## 💻 Protocolo de Comandos Serial (CLI em 9600 baud)

Abra o **Monitor Serial** da Arduino IDE configurado em **9600 baud** com final de linha em **Ambos NL & CR** ou apenas **Nova Linha (`\n`)**.

| Comando Enviado | Resposta da Placa | Descrição da Ação |
| :--- | :---: | :--- |
| `cabo 0` | `OK` | Desliga todos os MOSFETs (Circuito aberto total) |
| `cabo 1` | `OK` | Ativa S1 ($4700\,\Omega$ - Desconectado nominal) |
| `cabo 2` | `OK` | Ativa S2 ($1500\,\Omega$ - Cabo de 13 A) |
| `cabo 3` | `OK` | Ativa S3 ($680\,\Omega$ - Cabo de 20 A) |
| `cabo 4` | `OK` | Ativa S4 ($220\,\Omega$ - Cabo de 32 A) |
| `cabo 5` | `OK` | Ativa S5 ($100\,\Omega$ - Cabo de 63 A / 70 A) |
| `cabo 6` | `OK` | Ativa S6 ($47\,\Omega$ - Simulação de curto) |
| `status` ou `cabo status` | Relatório completo | Imprime canal ativo, resistência em $\Omega$, corrente em A e descrição |
| `help` ou `?` | Menu de ajuda | Exibe a lista completa de comandos aceitos |
| `<comando invalido>` | `ERRO` ou `ERRO: Cabo` | Tratamento seguro de entrada incorreta |

---

## ⚡ Como Gravar

1. Conecte o cabo USB ao Arduino (Nano, Uno ou Mega 2560);
2. Na Arduino IDE:
   - Selecione a placa correspondente em `Ferramentas` -> `Placa`;
   - Se for Arduino Nano clone (CH340), selecione `Processador: ATmega328P (Old Bootloader)`;
   - Selecione a porta COM correspondente em `Ferramentas` -> `Porta`;
3. Clique em **Carregar** (`Ctrl + U`).

