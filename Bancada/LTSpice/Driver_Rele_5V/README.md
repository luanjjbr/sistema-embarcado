---
tags:
  - hardware
  - ltspice
  - rele
  - s8050
  - 1n4007
  - driver
aliases:
  - Driver Rele S8050
  - Modulo Rele Songle
---

# 🔌 Driver de Acionamento para Relé de 5V (SRD-05VDC-SL-C / S8050)

Circuito de acionamento para relé eletromecânico Songle de $5\text{ V}$, baseado na topologia comercial com transistor NPN **S8050** e diodo de proteção **1N4007**.

---

## 1. Especificações dos Componentes da Imagem
- **Transistor Q2:** NPN S8050M-D ($V_{CEO} = 25\text{ V}$, $I_C = 700\text{ mA}$, ganho $h_{FE} \approx 200$);
- **Resistor de Base R2:** $1\text{ k}\Omega$ (corrente de base $I_B \approx 4{,}3\text{ mA}$, garantindo saturação total);
- **Resistor de Pull-Down $R_{PD}$:** $10\text{ k}\Omega$ (protege a base contra disparos espúrios no boot/reset);
- **Diodo D1:** 1N4007 ($1\text{ A}$, $1000\text{ V}$) em antiparalelo com a bobina (elimina a sobretensão $-L \frac{di}{dt}$);
- **Relé RLY1:** Songle SRD-05VDC-SL-C ($R = 70\,\Omega$, $L = 100\text{ mH}$, contatos $10\text{ A} / 250\text{ VAC}$);
- **Pinagem dos Contatos:** `C_1` (Comum), `NC_1` (Normalmente Fechado) e `NO_1` (Normalmente Aberto).

---

## 2. Resultados da Simulação no LTspice
- **Tensão de Saturação ($V_{CE(sat)}$):** $< 0{,}15\text{ V}$ em condução;
- **Corrente de Regime na Bobina:** $\approx 69{,}3\text{ mA}$;
- **Grampeamento de Sobretensão no Desligamento:** $V_{COL(max)} \le 5{,}7\text{ V}$ (seguro para o S8050).
