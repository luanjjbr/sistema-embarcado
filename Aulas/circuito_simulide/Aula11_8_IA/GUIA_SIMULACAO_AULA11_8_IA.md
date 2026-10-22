# 🚀 Guia de Simulação no SimulIDE: Aula 11_8 IA (Editor Numérico de 6 Dígitos)

Este projeto implementa uma interface homem-máquina (IHM) profissional com o **Arduino Mega 2560**, display **LCD 16x2 HD44780**, encoder rotativo **KY-040** e **FreeRTOS**.

---

## 🎮 1. Instruções de Operação da Interface

O editor manipula um número de 6 dígitos (`000000` a `999999`) e opera em **2 Modos**:

### Modo 1: Navegação de Casas (`NAV`)
* **Girar Horário (CW):** Move o cursor para a direita (próxima casa). Se atingir a última casa (unidade), **trava no fim e não passa**.
* **Girar Anti-horário (CCW):** Move o cursor para a esquerda (casa anterior). Se atingir a primeira casa (centena de milhar), **trava no início e não passa**.
* **Clique Curto no Botão (SW):** Seleciona a casa decimal atual e entra no **Modo Edição**.
* **Clique Longo no Botão (SW > 1.2s):** Reseta o número imediatamente para `000000`.

### Modo 2: Edição da Casa Selecionada (`EDIT`)
* O cursor do display LCD fica **piscando (*blink*)** sobre o dígito em alteração.
* **Girar Horário (CW):** Adiciona o peso daquela casa decimal (+1, +10, +100, +1.000, +10.000, +100.000). Se atingir `999999`, **trava no limite superior sem estourar**.
* **Girar Anti-horário (CCW):** Subtrai o peso decimal. Se atingir `0`, **trava no zero e não fica negativo**.
* **Clique Curto no Botão (SW):** Confirma o valor, desmarca a casa e volta ao **Modo Navegação**.

---

## ▶️ 2. Como Executar no SimulIDE

1. No SimulIDE, abra o circuito:
   ```text
   Aulas/circuito_simulide/Aula11_8_IA/circuito_aula11_8_ia.sim1
   ```
2. O circuito já está configurado para buscar o binário compilado em:
   ```text
   ../../codigos_aulas/Aula11_8_IA/build/arduino.avr.mega/Aula11_8_IA.ino.hex
   ```
3. Clique no botão de **Power** (vermelho/verde) no topo para iniciar.
4. Clique com o botão direito na placa Arduino Mega -> selecione **Open Serial Monitor** (9600 baud) para acompanhar os eventos em tempo real.
5. Gire o encoder e clique no botão para testar a navegação e a edição!

