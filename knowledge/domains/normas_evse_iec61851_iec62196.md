# ⚡ Domínio: Normas de Recarga Modo 3 (IEC 61851-1 / IEC 62196-1/2)

Consolidado a partir dos padrões IEC e materiais acadêmicos do GEPOC/UFSM.

---

## 1. Modos de Recarga de Veículos Elétricos (IEC 61851-1)
- **Modo 1:** Conexão AC direta à rede sem sinalização de segurança (descontinuado em muitas regiões por risco elétrico).
- **Modo 2:** Conexão AC à rede padrão com dispositivo de proteção e controle incorporado no cabo (IC-CPD).
- **Modo 3:** Recarga AC dedicada com Estação de Recarga (EVSE) fixa, com piloto de controle (CP) e detecção de proximidade (PP).
- **Modo 4:** Recarga DC rápida em alta potência com conversor off-board.

---

## 2. Pino PP (Proximity Pilot) — IEC 62196-2
Informa a capacidade nominal de condução de corrente do cabo conectado entre VE e EVSE através de resistores medidos para o condutor de proteção (PE).

| Estado / Capacidade | Faixa Tolerada (Norma) | Resistor Nominal Projetado |
| :--- | :--- | :--- |
| **Desconectado** | $> 4500\,\Omega$ | $4700\,\Omega$ |
| **Cabo 13 A** | $1100\,\Omega \text{ a } 2460\,\Omega$ | $1500\,\Omega$ |
| **Cabo 20 A** | $400\,\Omega \text{ a } 936\,\Omega$ | $680\,\Omega$ |
| **Cabo 32 A** | $164\,\Omega \text{ a } 308\,\Omega$ | $220\,\Omega$ |
| **Cabo 63 A / 70 A** | $80\,\Omega \text{ a } 140\,\Omega$ | $100\,\Omega$ |
| **Falha / Curto-circuito** | $< 60\,\Omega$ | $47\,\Omega$ |

*Requisito Obrigatório:* Intertravamento elétrico **Break-Before-Make** nas chaves de comutação para evitar associação espúria de resistores em paralelo durante transições.

---

## 3. Pino CP (Control Pilot) — IEC 61851-1
Barramento analógico e PWM bidirecional ($\pm 12\text{ V}$, $1\text{ kHz}$) com diodo em série ($D_1$) no veículo elétrico.

### 3.1. Níveis de Tensão Positiva ($V_A$) e Estados
- **Estado A ($12\text{ V}$ nom, $11{,}4\text{ V a } 12{,}6\text{ V}$):** VE desconectado (circuito aberto).
- **Estado B ($9\text{ V}$ nom, $8{,}37\text{ V a } 9{,}59\text{ V}$):** VE conectado, não pronto para receber carga ($R_2 = 2{,}74\text{ k}\Omega$).
- **Estado C ($6\text{ V}$ nom, $5{,}47\text{ V a } 6{,}53\text{ V}$):** VE conectado e pronto para carga, sem exigência de ventilação ($R_2 \parallel R_3 = 882\,\Omega$ com $R_3 = 1{,}3\text{ k}\Omega$).
- **Estado D ($3\text{ V}$ nom, $2{,}0\text{ V a } 4{,}0\text{ V}$):** VE conectado e pronto para carga, com ventilação exigida ($R_2 \parallel R_3 = 246\,\Omega$ com $R_3 = 270\,\Omega$).
- **Estado E ($0\text{ V}$ nom, $< 1{,}0\text{ V}$):** Falha / Curto-circuito CP-PE / Falha de aterramento.
- **Estado F ($-12\text{ V}$ contínuo):** Falha ou indisponibilidade da estação.

### 3.2. Relação Duty Cycle (%) vs. Corrente Máxima
- $D < 8\%$: Recarga não permitida.
- $8\% \le D \le 10\%$: Corrente fixa de $6\text{ A}$.
- $10\% < D \le 85\%$: Corrente máxima = $D \times 0{,}6\text{ A}$.
- $85\% < D \le 96\%$: Corrente máxima = $(D - 64) \times 2{,}5\text{ A}$.
- $D > 96\%$: Inválido / Falha.

### 3.3. Requisitos Críticos de Segurança e Temporização
- **Desconexão Rápida:** Em caso de desconexão do cabo ou transição para Estado E (falha de terra/curto), a contatora da EVSE deve abrir em menos de **$100\text{ ms}$**.
- **Detecção do Diodo $D_1$:** A parcela negativa da forma de onda deve manter-se em $-12\text{ V}$. Se a tensão negativa subir (ex: divisor resistivo sem diodo), a EVSE deve identificar ausência de veículo elétrico e abortar o fornecimento.
