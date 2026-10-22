# 🚀 Guia de Simulação no SimulIDE: Aula 11 (Arduino Mega + Encoder KY-040)

Este projeto simula exclusivamente o encoder rotativo incremental **KY-040** com o **Arduino Mega 2560** e tarefas multitarefa no **FreeRTOS** (`TaskBlink`, `TaskUART`, `TaskENCODERRead`).

---

## 🔌 1. Conexões do Circuito

| Componente | Pino do Módulo | Pino Arduino Mega | Finalidade no Firmware |
| :--- | :--- | :--- | :--- |
| **Encoder KY-040** | **CLK** | **Pino 11** | Canal A de quadratura (`PB5` / `PCINT5`) |
| **Encoder KY-040** | **DT** (CCLK) | **Pino 12** | Canal B de quadratura (`PB6` / `PCINT6`) |
| **Encoder KY-040** | **SW** | **Pino 10** | Botão central (`PB4` / `PCINT4`) |
| **Encoder KY-040** | **GND** | **Pino 8** | Nível lógico `LOW` (Alimentação ativa) |
| **Encoder KY-040** | **+ (VCC)** | **Pino 9** | Nível lógico `HIGH` (Alimentação ativa) |

---

## ▶️ 2. Como Executar no SimulIDE

1. No SimulIDE, abra o arquivo:
   ```text
   Aulas/circuito_simulide/Aula11/circuito_aula11.sim1
   ```
2. O firmware compilado `Aula11.ino.hex` já está vinculado automaticamente com `Auto_Load="true"`.
3. Clique no botão de **Power** (vermelho/verde) no topo para ligar o circuito.
4. **Abra o Monitor Serial:**
   - Clique com o botão direito na placa Arduino Mega -> clique em **Open Serial Monitor**.
   - Configure o baud rate para **9600 baud**.
5. Gire o encoder com o mouse ou clique no botão central (SW) para visualizar os logs de status na UART:
   ```text
   Estado: 1, 0, 0, CK: 2, 0, 0
   ```
   - `Estado`: 0 (Pressionado) / 1 (Solto)
   - `CK`: 0 (Direita - D) / 1 (Esquerda - E) / 2 (Parado - P)

