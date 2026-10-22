# 📘 Guia de Simulação — Aula 11_8 HW (Pinagem Real PEC11R)

Este projeto implementa a leitura por interrupção na **borda de descida nativa** para a pinagem real do esquemático do hardware:
- **`CLK`:** Pino **10** (`PB4` / `PCINT4`)
- **`DT`:** Pino **9** (`PH6` / I/O digital)
- **`SW`:** Pino **8** (`PH5` / I/O digital com pull-up)
- **`LCD 16x2`:** Pinos 23, 25, 27, 29 (dados) e 39, 41, 43 (controle).

---

## 🛠️ Como Funciona a Interrupção por Borda de Descida Nativa

1. No registrador `PCMSK0`, **apenas o bit 4 (Pino 10)** está habilitado:
   ```cpp
   PCICR  |= (1 << PCIE0);
   PCMSK0  = (1 << PCINT4);
   ```
2. A interrupção `ISR(PCINT0_vect)` só acorda quando o pino 10 varia.
3. Se o pino 10 estiver em `LOW`, confirma-se a borda de descida (`1 -> 0`), dispensando qualquer variável de histórico em software!

---

## 🚀 Como Executar no SimulIDE

1. Abra o **SimulIDE**.
2. Abra o circuito: `Aulas/circuito_simulide/Aula11_8_HW/circuito_aula11_8_hw.sim1`.
3. O firmware já está vinculado automaticamente a:
   `../../codigos_aulas/Aula11_8_HW/build/arduino.avr.mega/Aula11_8_HW.ino.hex`
4. Pressione o botão **Power (Vermelho -> Amarelo/Verde)** no SimulIDE.
5. Gire o knob do encoder:
   - Para a direita: o LCD e a Serial mostram `DIR: DIREITA` e incrementam `VALOR`.
   - Para a esquerda: o LCD e a Serial mostram `DIR: ESQUERDA` e decrementam `VALOR`.
6. Pressione o botão do encoder: o status `BT:` muda para `1` e `SW: PRESS`.

