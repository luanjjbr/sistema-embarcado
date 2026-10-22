# 🚀 Guia de Simulação no SimulIDE: Aula 11_8 (Arduino Mega + LCD + Encoder)

Este guia orienta passo a passo como executar a simulação do firmware da **Aula 11_8** utilizando o **SimulIDE**.

---

## 📋 1. Mapeamento Completo de Conexões (Pinagem)

### 1.1 Display LCD 16x2 (Controlador HD44780)
| Pino LCD | Função | Pino Arduino Mega | Finalidade no Código |
| :--- | :--- | :--- | :--- |
| **1 (VSS)** | Ground (GND) | **Pino 49** | Nível lógico `LOW` (`digitalWrite(ipinVSS, LOW)`) |
| **2 (VDD)** | Alimentação (+5V) | **Pino 47** | Nível lógico `HIGH` (`digitalWrite(ipinVDD, HIGH)`) |
| **3 (V0)** | Contraste | **Pino 45** | PWM `analogWrite(ipinV0, 64)` (ajuste de contraste) |
| **4 (RS)** | Register Select | **Pino 43** | Seleção entre comando ou dado |
| **5 (RW)** | Read / Write | **Pino 41** | Modo de escrita (`digitalWrite(ipinRW, LOW)`) |
| **6 (E)** | Enable | **Pino 39** | Pulso de clock para validação dos dados |
| **11 (DB4)** | Barramento Dados bit 4 | **Pino 29** | Linha de dados do nibble |
| **12 (DB5)** | Barramento Dados bit 5 | **Pino 27** | Linha de dados do nibble |
| **13 (DB6)** | Barramento Dados bit 6 | **Pino 25** | Linha de dados do nibble |
| **14 (DB7)** | Barramento Dados bit 7 | **Pino 23** | Linha de dados do nibble |
| **15 (BLA/15)** | Backlight Catodo | **Pino 51** | Nível lógico `LOW` (`digitalWrite(ipin15, LOW)`) |
| **16 (BLK/16)** | Backlight Anodo | **Pino 53** | Nível lógico `HIGH` (`digitalWrite(ipin16, HIGH)`) |

> [!NOTE]
> No SimulIDE, o componente **Hd44780** pode ser alimentado diretamente pelos pinos de controle ou aterrado ao GND comum para simplificar a visualização.

---

### 1.2 Módulo Encoder Rotativo Incremental (KY-040)
| Pino KY-040 | Função | Pino Arduino Mega | Modo / Registrador AVR |
| :--- | :--- | :--- | :--- |
| **GND** | Referência 0V | **Pino 8** | Saída digital em nível `LOW` |
| **+ (VCC)** | Alimentação (+5V) | **Pino 9** | Saída digital em nível `HIGH` |
| **SW** | Botão Central de Clique | **Pino 10** | Entrada com interrupção `PB4` (`PCINT4`) |
| **CLK** | Canal A (Clock) | **Pino 11** | Entrada com interrupção `PB5` (`PCINT5`) |
| **DT (CCLK)**| Canal B (Dados/Direção) | **Pino 12** | Entrada com interrupção `PB6` (`PCINT6`) |

---

## ⚙️ 2. Como Gerar o Arquivo Binário Compilado (`.hex`)

O SimulIDE executa o código de máquina diretamente no microcontrolador emulado (**ATmega2560**).

1. Abra o arquivo `Aula11_8.ino` (ou `teste.ino`) no **Arduino IDE**.
2. No menu **Ferramentas -> Placa -> Arduino AVR Boards**, selecione **Arduino Mega or Mega 2560**.
3. No menu **Ferramentas -> Processador**, certifique-se de que está selecionado **ATmega2560 (Mega 2560)**.
4. Pressione o atalho:
   ```text
   Ctrl + Alt + S
   ```
   *(Ou acesse pelo menu: **Sketch -> Exportar Binário Compilado**)*.
5. O Arduino IDE compilará o sketch e criará o arquivo binário com final `.hex` (ex.: `Aula11_8.ino.hex` ou `Aula11_8.ino.with_bootloader.hex`) na pasta do sketch ou na pasta de build.

---

## 🖥️ 3. Como Abrir e Executar a Simulação no SimulIDE

1. **Inicie o SimulIDE**:
   - Abra o executável `simulide.exe`.
2. **Carregue o Circuito**:
   - Clique no ícone de pasta na barra superior (**Open Circuit**) ou pressione `Ctrl + O`.
   - Navegue até esta pasta:
     ```text
     Aulas/circuito_simulide/Aula11_8/
     ```
   - Selecione o arquivo `circuito_aula11_8.simu` (ou `circuito_aula11_8.sim1` se estiver em versões 1.0+).
3. **Carregue o Firmware no Arduino Mega**:
   - Clique com o **botão direito do mouse** em cima da placa **Arduino Mega** (ou do microcontrolador ATmega2560).
   - No menu de contexto que abrir, clique em **Load Firmware**.
   - Selecione o arquivo `.hex` exportado no Passo 2.
4. **Ligue o Circuito**:
   - Clique no botão vermelho de **Power** (ícone de liga/desliga no topo do SimulIDE). Ele ficará verde indicando que a simulação está rodando em tempo real.
5. **Abra o Monitor Serial**:
   - Clique com o botão direito na placa Arduino Mega -> selecione **Open Serial Monitor**.
   - Configure o baud rate para **9600 baud**.
6. **Interaja com o Encoder e LCD**:
   - Clique e arraste o dial do componente **Encoder KY-040** para a direita ou para a esquerda.
   - Veja o valor de `Valor` e os tempos sendo transmitidos na UART e o número atualizando dinamicamente no display **LCD 16x2**!
   - Clique no botão central (**SW**) para testar a detecção de pressão e liberação.

