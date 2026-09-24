# 🔌 Domínio: Emulação do Pino Proximity Pilot (IEC 61851-1 / IEC 62196-2)

## 1. Finalidade
Emular a conexão do cabo de recarga veicular Tipo 2 através da seleção de resistores no pino PP (Proximity Pilot).

## 2. Tabela de Resistências e Canais
| Canal | Pino Arduino | Resistor Nominal | Estado do Cabo / Capacidade |
| :---: | :---: | :---: | :--- |
| **S0** | Nenhum | Aberto ($\infty$) | Desconectado Total |
| **S1** | D4 | $4700\,\Omega$ | Desconectado Nominal |
| **S2** | D5 | $1500\,\Omega$ | Cabo conectado — Capacidade **13 A** |
| **S3** | D6 | $680\,\Omega$ | Cabo conectado — Capacidade **20 A** |
| **S4** | D7 | $220\,\Omega$ | Cabo conectado — Capacidade **32 A** |
| **S5** | D8 | $100\,\Omega$ | Cabo conectado — Capacidade **63 A** |
| **S6** | D9 | $47\,\Omega$ | Simulação de Falha / Curto-circuito |

## 3. Intertravamento Break-Before-Make
- Ao trocar de canal, a chave ativa anterior deve ser desacionada **antes** do acionamento da nova chave (`desativarTodos()` -> `acionarCanal(novo)`), com pequeno intervalo de acomodação para evitar resistências espúrias em paralelo.
