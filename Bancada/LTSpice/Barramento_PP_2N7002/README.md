---
tags:
  - hardware
  - ltspice
  - iec62196
  - proximity-pilot
  - mosfet
  - 2n7002
  - gate-drive
aliases:
  - Barramento PP 2N7002 com Gate Drive
  - Emulador PP MOSFET Completo
---

# 🔌 Barramento de Resistores PP (IEC 62196-2) com Gate Drive 2N7002

Esquemático de precisão para emulação da capacidade de condução de corrente do cabo de recarga veicular (Pino PP). Todos os 6 canais contam com transistores MOSFET canal N **2N7002** e malhas dedicadas de acionamento de gate.

---

## 1. Arquitetura de Gate Drive em Cada Canal
- **Resistor de Amortecimento ($R_{G} = 220\,\Omega$):** Limita a corrente de pico de carga do gate e elimina oscilações parasitas de alta frequência;
- **Resistor de Pull-Down ($R_{PD} = 10\text{ k}\Omega$):** Drena cargas estáticas para o terra PE, mantendo o MOSFET em corte durante boot e reset do microcontrolador;
- **Topologia Low-Side:** Todos os Sources aterrados diretamente no barramento PE ($0\text{ V}$), garantindo saturação perfeita com sinal lógico de $5\text{ V}$.

---

## 2. Tabela de Tolerâncias e Valores Equivalentes

| Canal | Estado Emulado | Resistor Nominal | Faixa Tolerada IEC | Resistência com $R_{DS(on)}$ |
| :---: | :--- | :---: | :---: | :---: |
| **S1** | Desconectado Nominal | $4700\,\Omega$ | $> 4500\,\Omega$ | $\approx 4702\,\Omega$ |
| **S2** | Cabo de 13 A | $1500\,\Omega$ | $1100\,\Omega \text{ a } 2460\,\Omega$ | $\approx 1502\,\Omega$ |
| **S3** | Cabo de 20 A | $680\,\Omega$ | $400\,\Omega \text{ a } 936\,\Omega$ | $\approx 682\,\Omega$ |
| **S4** | Cabo de 32 A | $220\,\Omega$ | $164\,\Omega \text{ a } 308\,\Omega$ | $\approx 222\,\Omega$ |
| **S5** | Cabo de 63 A / 70 A | $100\,\Omega$ | $80\,\Omega \text{ a } 140\,\Omega$ | $\approx 102\,\Omega$ |
| **S6** | Falha / Curto | $47\,\Omega$ | $< 60\,\Omega$ | $\approx 49\,\Omega$ |
