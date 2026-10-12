---
tags:
  - hardware
  - ltspice
  - mosfet
  - 2n7002
  - gate-drive
aliases:
  - Driver 2N7002
  - Chave MOSFET PP
---

# ⚡ Circuito de Acionamento (Gate Drive) — MOSFET 2N7002

Ambiente de teste e caracterização para comutação das resistências do **Pino PP (IEC 62196-2)** utilizando o MOSFET canal N **2N7002** em substituição a chaves ideais.

---

## 1. Topologia do Circuito
- **MOSFET:** 2N7002 (N-Channel, $60\text{ V}$, $115\text{ mA}$, $R_{DS(on)} \approx 2\,\Omega$);
- **Resistor de Gate ($R_{GATE}$):** $220\,\Omega$ (amortece transitórios de corrente capacitiva);
- **Pull-down de Gate ($R_{PD}$):** $10\text{ k}\Omega$ (mantém $V_{GS} = 0\text{ V}$ no boot/reset);
- **Carga de Teste:** $R_{PP} = 680\,\Omega$ (Cabo 20 A) com pull-up de $470\,\Omega$ em $5\text{ V}$.

---

## 2. Resultados Esperados da Simulação
- **$V_{GS}$ em condução:** $5\text{ V}$ (chave plenamente saturada);
- **Corrente de Dreno ($I_D$):** $\approx 4{,}3\text{ mA}$;
- **Queda Dreno-Fonte ($V_{DS}$):** $< 10\text{ mV}$;
- **Impacto na precisão do cabo:** O $R_{DS(on)}$ de $\approx 2\,\Omega$ representa menos de $0{,}3\%$ do valor de $680\,\Omega$, mantendo o cabo rigorosamente dentro da faixa de tolerância da norma IEC ($400\,\Omega \text{ a } 936\,\Omega$).
