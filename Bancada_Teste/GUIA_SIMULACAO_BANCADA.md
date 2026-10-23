# Guia de Simulação — Bancada de Testes EV no SimulIDE 1.1.0-SR2

Este guia orienta a integração e o teste completo da **Bancada de Testes de Cabos (IEC 62196-2)** no **SimulIDE** com o aplicativo desktop **.NET 10** (*Bancada de Testes EVSE*), incluindo o barramento eletrônico com transistores MOSFET e voltímetro analógico.

---

## 🌐 1. Topologia de Conexão Virtual (COM1 ↔ COM2)

```text
[ Aplicativo .NET 10 ]  <--->  ( COM2 )  <=== [Túnel Virtual] ===>  ( COM1 )  <--->  [ SimulIDE: BRIDGE_COM1 ]
 (Selecionar COM2)                                                                       (Arduino Uno FreeRTOS)
```

O circuito virtual integra:
1. **Ponte Serial (BRIDGE_COM1):** Conectada à porta virtual `COM1` a 9600 bps com auto-conexão ativada;
2. **Sinalização Visual (PP_STATUS):** Barra de 8 LEDs horizontal mostrando os canais D4..D9 e o *Heartbeat* do FreeRTOS em D13;
3. **Barramento Físico do Cabo PP (IEC 62196-2):**
   - 6 MOSFETs 2N7002 ($Q_1$ a $Q_6$) chaveando os resistores nominais ($4700\,\Omega$, $1500\,\Omega$, $680\,\Omega$, $220\,\Omega$, $100\,\Omega$ e $47\,\Omega$);
   - Circuito de detecção da estação EVSE: Fonte de $+5\text{ V}$ e resistor de pull-up $R_{PU} = 470\,\Omega$;
   - **Voltímetro Probe (`V_PP`):** Mostra a tensão analógica exata no pino PP em tempo real.

---

## 🛠️ 2. Passo a Passo de Execução

### Passo 1: Iniciar o Circuito no SimulIDE
1. Abra o **SimulIDE** (`SimulIDE.exe`);
2. Pressione `Ctrl + O` e selecione:
   ```text
   C:\Users\italo\OneDrive\Imagens\Git\sistema-embarcado\Bancada_Teste\circuito_bancada_teste.sim1
   ```
3. Clique no botão vermelho **Power Circuit** (topo esquerdo);
4. O SimulIDE se conectará automaticamente à **`COM1`** e o **LED 7 (D13)** começará a piscar suavemente (*Heartbeat* do FreeRTOS);
5. O voltímetro **`V_PP (IEC 62196)`** exibirá inicialmente **`5.00 V`** (circuito aberto).

---

### Passo 2: Conectar seu Aplicativo .NET 10
1. Abra o seu aplicativo desktop **Bancada de Testes EVSE**;
2. No seletor **PORTA SERIAL**, escolha **`COM2`**;
3. No campo **BAUD RATE**, mantenha **`9600`**;
4. Clique em **Conectar**. O status mudará para verde (*Conectado*).

---

### Passo 3: Ensaios e Validação em Tempo Real

Ao clicar nas opções de cabo no aplicativo .NET, observe simultaneamente a resposta serial, o LED ativo e a tensão no voltímetro:

| Opção no Aplicativo .NET | Resistor ($R_{PP}$) | LED Ativo | Corrente Máxima | Tensão no Voltímetro ($V_{PP}$) |
| :--- | :---: | :---: | :---: | :---: |
| **0: Circuito Aberto** | $\infty$ | Nenhum | 0 A | **$5{,}00\text{ V}$** |
| **1: Desconectado** | $4700\,\Omega$ | **LED 0** (`D4`) | 0 A | **$\approx 4{,}54\text{ V}$** |
| **2: Cabo 13 A** | $1500\,\Omega$ | **LED 1** (`D5`) | **13 A** | **$\approx 3{,}80\text{ V}$** |
| **3: Cabo 20 A** | $680\,\Omega$ | **LED 2** (`D6`) | **20 A** | **$\approx 2{,}95\text{ V}$** |
| **4: Cabo 32 A** | $220\,\Omega$ | **LED 3** (`D7`) | **32 A** | **$\approx 1{,}59\text{ V}$** |
| **5: Cabo 63 A** | $100\,\Omega$ | **LED 4** (`D8`) | **63 A** | **$\approx 0{,}87\text{ V}$** |
| **6: Falha / Curto** | $47\,\Omega$ | **LED 5** (`D9`) | 0 A | **$\approx 0{,}45\text{ V}$** |

---

## 🔧 3. Mapeamento de Pinos e Componentes

- **`D4`**: Gate de $Q_1$ (2N7002) + LED 0 $\rightarrow$ Resistor $4700\,\Omega$
- **`D5`**: Gate de $Q_2$ (2N7002) + LED 1 $\rightarrow$ Resistor $1500\,\Omega$ (13 A)
- **`D6`**: Gate de $Q_3$ (2N7002) + LED 2 $\rightarrow$ Resistor $680\,\Omega$ (20 A)
- **`D7`**: Gate de $Q_4$ (2N7002) + LED 3 $\rightarrow$ Resistor $220\,\Omega$ (32 A)
- **`D8`**: Gate de $Q_5$ (2N7002) + LED 4 $\rightarrow$ Resistor $100\,\Omega$ (63 A)
- **`D9`**: Gate de $Q_6$ (2N7002) + LED 5 $\rightarrow$ Resistor $47\,\Omega$ (Falha/Curto)
- **`D13`**: LED 7 $\rightarrow$ Heartbeat FreeRTOS (500 ms)
- **`D0/D1`**: Interface UART ligada à ponte `BRIDGE_COM1`
- **`V_PP`**: Linha central com pull-up de $470\,\Omega$ para $+5\text{ V}$ e probe analógico
