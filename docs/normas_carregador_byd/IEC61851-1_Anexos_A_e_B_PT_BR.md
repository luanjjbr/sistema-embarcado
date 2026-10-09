# NORMA INTERNACIONAL IEC 61851-1 (Edição 2.0 / 2010-11)
## Sistema de Recarga Condutiva para Veículos Elétricos — Parte 1: Requisitos Gerais
### Tradução Técnica de Referência: Anexo A (Normativo) e Anexo B (Informativo)

---

# ANEXO A (Normativo)
## Função Piloto Através de Circuito de Controle Piloto Utilizando Modulação PWM e Condutor Piloto

### A.1 Generalidades
Este anexo aplica-se a todos os sistemas de recarga que implementam a função piloto com um circuito de condutor piloto com modulação por largura de pulso (PWM), a fim de definir o nível de corrente disponível para recarga em **Modo 2** e **Modo 3**. 
Este anexo descreve as funções e o sequenciamento de eventos para este circuito com base nos parâmetros recomendados de implementação típica. Os parâmetros indicados foram escolhidos para assegurar a interoperabilidade de sistemas com aqueles projetados de acordo com a norma **SAE J1772**.

*NOTA:* Este anexo não é aplicável a veículos que utilizem funções piloto não baseadas em sinal PWM e condutor piloto.

---

### A.2 Circuito de Controle Piloto
Os parâmetros do circuito são definidos nas Tabelas A.1, A.2, A.3, A.5, A.6 e A.7.

#### Circuito Típico do VE (Veículo Elétrico):
- Condutor CP conectado ao anodo do diodo $D_1$.
- Catodo de $D_1$ conectado a $R_2 = 2{,}74\text{ k}\Omega$ conectado ao terra de proteção (PE/Chassi).
- Chave $S_2$ conectando $R_3 = 1{,}3\text{ k}\Omega$ (sem ventilação) ou $R_3 = 270\,\Omega$ (com ventilação) em paralelo com $R_2$.

#### Tabela A.1 — Parâmetros do Circuito de Controle Piloto da Estação (EVSE)
| Parâmetro | Símbolo | Valor | Unidade |
| :--- | :---: | :---: | :---: |
| Tensão positiva de circuito aberto do gerador | $V_{och}$ | $12{,}00\ (\pm 0{,}6)$ | V |
| Tensão negativa de circuito aberto do gerador | $V_{ocl}$ | $-12{,}00\ (\pm 0{,}6)$ | V |
| Frequência do oscilador | $F_o$ | $1\,000\ (\pm 0{,}5\%)$ | Hz |
| Largura de pulso | $P_{wo}$ | Conforme Tabela A.4 $(\pm 25\,\mu\text{s})$ | $\mu\text{s}$ |
| Tempo de subida máximo ($10\%$ a $90\%$) | $T_{rg}$ | $2$ | $\mu\text{s}$ |
| Tempo de descida máximo ($90\%$ a $10\%$) | $T_{fg}$ | $2$ | $\mu\text{s}$ |
| Tempo mínimo de acomodação para $95\%$ de regime estável | $T_{sg}$ | $3$ | $\mu\text{s}$ |
| Resistência de fonte equivalente | $R_1$ | $1\,000 \pm 3\%$ | $\Omega$ |
| Supressão de EMI recomendada | $C_s$ | $300$ | pF |
| Capacitância total máxima do cabo $+ C_s$ | $C_s + C_c$ | $3\,100$ | pF |

---

#### Tabela A.2 — Valores e Parâmetros do Circuito Piloto no Veículo Elétrico
| Parâmetro | Símbolo | Valor | Unidade |
| :--- | :---: | :---: | :---: |
| Resistor permanente do veículo | $R_2$ | $2{,}74\text{ k}\ (\pm 3\%)$ | $\Omega$ |
| Resistor comutável (sem ventilação exigida) | $R_3$ | $1{,}3\text{ k}\ (\pm 3\%)$ | $\Omega$ |
| Resistor comutável (com ventilação exigida) | $R_3$ | $270\ (\pm 3\%)$ | $\Omega$ |
| Resistência equivalente total sem ventilação ($R_2 \parallel R_3$) | $R_e$ | $882\ (\pm 3\%)$ | $\Omega$ |
| Resistência equivalente total com ventilação ($R_2 \parallel R_3$) | $R_e$ | $246\ (\pm 3\%)$ | $\Omega$ |
| Queda de tensão direta do diodo $D_1$ ($2{,}75\text{ a } 10\text{ mA}$, $-40^\circ\text{C a }+85^\circ\text{C}$) | $V_d$ | $0{,}7\ (\pm 0{,}15)$ | V |
| Capacitância equivalente máxima de entrada do veículo | $C_v$ | $2\,400$ | pF |

---

#### Tabela A.3 — Estados do Veículo e Funções Piloto
| Estado do VE | VE Conectado | Chave $S_2$ | Recarga Possível | Tensão Positiva Medida ($V_A$) | Descrição e Ação |
| :---: | :---: | :---: | :---: | :---: | :--- |
| **A** | Não | Aberta | Não | $12\text{ V}$ estático ($\pm 1\text{ V}$) | VE Desconectado. Tensão estática do oscilador. |
| **B** | Sim | Aberta | Não | $9\text{ V}$ ($\pm 1\text{ V}$) | VE Conectado; $R_2$ detectado; VE não pronto para recarga. |
| **C** | Sim | Fechada | VE Pronto | $6\text{ V}$ ($\pm 1\text{ V}$) | VE Conectado; Pronto para recarga; Sem exigência de ventilação. |
| **D** | Sim | Fechada | VE Pronto | $3\text{ V}$ ($\pm 1\text{ V}$) | VE Conectado; Pronto para recarga; Ventilação da área de recarga obrigatória. |
| **E** | Sim | Aberta | Não | $0\text{ V}$ | Falha / Curto CP-PE; EVSE sem alimentação ou terra rompido. |
| **F** | Sim | Aberta | Não | $-12\text{ V}$ contínuo | EVSE não disponível ou falha interna na estação. |

---

### A.3 Sequência Típica de Conexão e Handshake

#### Tabela A.4 — Sequência Passo a Passo de Conexão e Recarga (Handshake)
| Passo | Estado | Descrição e Condições do Evento |
| :---: | :---: | :--- |
| **1** | **A** | **Veículo Desconectado:** A tensão total do gerador da EVSE é medida em $V_A$ ($+12\text{ V}$ DC contínuo). |
| **2** | **B** | **Cabo Conectado ao VE e à EVSE:** Condição detectada pelo sinal de $+9\text{ V}$ medido em $V_A$. O sinal do gerador ($V_g$) pode ser $+12\text{ V}$ DC estático (Estado B1) ou onda quadrada de $\pm 12\text{ V}$ / $1\text{ kHz}$ (Estado B2) se a EVSE estiver pronta para fornecer energia. |
| **3** | **B** | **EVSE Pronta para Fornecer Energia:** A EVSE transmite o PWM de $1\text{ kHz}$ indicando a corrente máxima pelo ciclo de trabalho (*duty cycle*). A presença do diodo $D_1$ é confirmada pela retenção de $-12\text{ V}$ na parcela negativa, garantindo que se trata de um VE real. |
| **4** | **B $\rightarrow$ C, D** | **Veículo Pronto para Receber Energia:** O veículo fecha a chave $S_2$. A tensão positiva cai de $+9\text{ V}$ para $+6\text{ V}$ (Estado C) ou $+3\text{ V}$ (Estado D). |
| **5** | **C, D** | **EVSE Fecha a Contatora:** A estação fecha os contatos de potência AC e energiza o cabo. |
| **6** | **C, D** | **Fluxo de Corrente de Carga:** O carregador on-board (OBC) do veículo drena corrente da rede, sem ultrapassar o limite indicado pelo *Duty Cycle*. |
| **7** | **C, D** | **Ajuste Dinâmico de Potência:** A EVSE pode alterar o *duty cycle* para reduzir ou aumentar a potência (gestão de rede). O veículo ajusta sua corrente em conformidade. |
| **8** | **C, D** | **Fim da Carga Decidido pelo Veículo:** Bateria totalmente carregada ou comando do usuário. |
| **9** | **C, D $\rightarrow$ B** | **Veículo Solicita Desconexão:** O veículo abre a chave $S_2$ (tensão retorna para $+9\text{ V}$). Pode ocorrer também pela liberação do botão do conector (Pino PP). |
| **10** | **B** | **Abertura da Contatora da EVSE:** A EVSE detecta o retorno para o Estado B e abre os contatos principais de potência em menos de $100\text{ ms}$. |
| **11** | **A** | **Remoção do Cabo:** Remoção completa do conector detectada pelo retorno da tensão para $+12\text{ V}$. A EVSE libera a trava mecânica do plugue. |

---

### A.4 Ciclo de Trabalho (Duty Cycle) do PWM e Capacidade de Corrente

#### Tabela A.5 — Ciclo de Trabalho Fornecido pela Estação (EVSE)
| Corrente Disponível da Rede | Ciclo de Trabalho Nominal (Tolerância $\pm 1\%$) |
| :--- | :--- |
| Comunicação digital para recarga rápida DC | $5\%$ Duty Cycle |
| Recarga proibida | $< 8\%$ |
| Corrente fixa de $6\text{ A}$ | $8\% \le \text{Duty} \le 10\%$ |
| Corrente entre $6\text{ A}$ e $51\text{ A}$ | $\text{Duty (\%)} = \dfrac{\text{Corrente [A]}}{0{,}6} \quad (10\% \le \text{Duty} \le 85\%)$ |
| Corrente entre $51\text{ A}$ e $80\text{ A}$ | $\text{Duty (\%)} = \left(\dfrac{\text{Corrente [A]}}{2{,}5}\right) + 64 \quad (85\% < \text{Duty} \le 96\%)$ |
| Recarga proibida | $> 97\%$ |

---

#### Tabela A.7 — Temporizações Críticas da Estação (EVSE) e do Veículo (VE)
| Símbolo | Valor Máximo | Descrição da Temporização |
| :--- | :---: | :--- |
| $t_1, t_{1a}$ | Sem máximo | Tempo para ativação do oscilador PWM de $1\text{ kHz}$ após detecção do Estado B. |
| $t_{ACon}$ | $3\text{ s}$ | Tempo para início do fornecimento de energia AC após a detecção do Estado C ou D. |
| $t_{external}$ | $10\text{ s}$ | Tempo de resposta para alteração do PWM após comando externo da rede/usuário. |
| $t_{ACoff1}$ | **$100\text{ ms}$** | **Tempo máximo para a contatora da EVSE abrir após abertura da chave $S_2$.** |
| $t_{ventilation}$ | $3\text{ s}$ | Atraso para ativação de ventilação após transição de C ($6\text{ V}$) para D ($3\text{ V}$). |
| $t_{abnormal}$ | $3\text{ s}$ | Tempo para abertura de contatos em condições anormais (tensão fora da tolerância). |
| $t_{proximity}$| **$100\text{ ms}$** | **Tempo máximo para abertura da contatora se o contato de proximidade (PP) for aberto.** |
| $t_{ichange}$ | $5\text{ s}$ | Tempo máximo para o VE ajustar a corrente após variação no *Duty Cycle* do PWM. |

---

# ANEXO B (Informativo)
## Exemplos de Diagramas de Circuito para Acoplador Básico e Universal

### B.1 Identificação dos Pinos do Conector Monofásico
- **1, 2:** Contatos de potência de Fase (L) e Neutro (N);
- **3:** Contato do condutor de terra e proteção elétrica (PE);
- **4:** Contato da função Controle Piloto (CP);
- **5:** Contato da função Detecção de Proximidade (PP).

---

### B.2 Codificação de Resistência do Pino PP (Proximity Pilot)
Os conectores e cabos que utilizam o contato de proximidade para codificação de capacidade de condução de corrente devem possuir um resistor $R_c$ conectado entre o pino PP e o condutor PE.

#### Tabela B.3 — Codificação de Resistores para Conjuntos de Cabos e Plugues
| Capacidade de Corrente do Cabo | Resistência Equivalente $R_c$ (Tolerância $\pm 3\%$) | Potência Nominal do Resistor |
| :---: | :---: | :---: |
| **$13\text{ A}$** | $1\,500\,\Omega\ (1{,}5\text{ k}\Omega)$ | $\ge 0{,}5\text{ W}$ |
| **$20\text{ A}$** | $680\,\Omega$ | $\ge 0{,}5\text{ W}$ |
| **$32\text{ A}$** | $220\,\Omega$ | $\ge 0{,}5\text{ W}$ |
| **$63\text{ A}$ (Trifásico) / $70\text{ A}$ (Monofásico)** | $100\,\Omega$ | $\ge 0{,}5\text{ W}$ |

*Requisito de Segurança:* A EVSE deve interromper imediatamente o fornecimento de corrente se a capacidade nominal do cabo for ultrapassada.
