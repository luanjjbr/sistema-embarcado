#include <Arduino_FreeRTOS.h>
#include <queue.h>
#include <LiquidCrystal.h>

// ==============================================================================
// CONFIGURAÇÃO DE PINAGEM (ARDUINO MEGA 2560)
// ==============================================================================
// Pinos do Display LCD 16x2 HD44780 (vindos da Aula 8)
const uint8_t ipinDB7 = 23, ipinDB6 = 25, ipinDB5 = 27, ipinDB4 = 29;
const uint8_t ipinE   = 39, ipinRW  = 41, ipinRS  = 43, ipinV0  = 45;
const uint8_t ipinVDD = 47, ipinVSS = 49, ipin15  = 51, ipin16  = 53;

// Pinos do Encoder KY-040 (vindos da Aula 11)
const uint8_t ci_pinEnc_GND  = 8;
const uint8_t ci_pinEnc_VCC  = 9;
const uint8_t ci_pinEnc_BTN  = 10; // SW (Botão de clique)
const uint8_t ci_pinEnc_CLK  = 11; // Canal A
const uint8_t ci_pinEnc_CCLK = 12; // Canal B (DT)

// Objeto de controle do display LCD
LiquidCrystal lcd(ipinRS, ipinRW, ipinE, ipinDB4, ipinDB5, ipinDB6, ipinDB7);

// ==============================================================================
// TIPOS E DEFINIÇÕES DO SISTEMA
// ==============================================================================
typedef enum {
  ENC_EVENT_NONE,
  ENC_EVENT_CW,         // Giro Horário
  ENC_EVENT_CCW,        // Giro Anti-horário
  ENC_EVENT_CLICK,      // Clique Curto (< 1.2s)
  ENC_EVENT_LONG_CLICK  // Clique Longo (> 1.2s - Reset)
} eEncoderEvent;

typedef enum {
  MODO_NAVEGACAO,       // Navega entre as casas decimais (0 a 5)
  MODO_EDICAO           // Altera o valor da casa selecionada
} eModoUI;

// Pesos decimais de cada uma das 6 casas (Casa 0 = 100.000 até Casa 5 = 1)
const uint32_t PESOS_DECIMAIS[6] = {
  100000UL, // Casa 0: Centena de milhar
   10000UL, // Casa 1: Dezena de milhar
    1000UL, // Casa 2: Unidade de milhar
     100UL, // Casa 3: Centena
      10UL, // Casa 4: Dezena
       1UL  // Casa 5: Unidade
};

const uint32_t VALOR_MAXIMO = 999999UL;
const uint32_t VALOR_MINIMO = 0UL;

// Fila FreeRTOS para transporte seguro de eventos do Encoder para a UI
QueueHandle_t xEncoderQueue = NULL;

// Protótipos das Tarefas FreeRTOS
void TaskEncoder(void *pvParameters);
void TaskUI(void *pvParameters);
void TaskHeartbeat(void *pvParameters);

// ==============================================================================
// SETUP E INICIALIZAÇÃO
// ==============================================================================
void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ;
  }

  // Alimentação ativa dos trilhos do LCD
  pinMode(ipinVDD, OUTPUT); digitalWrite(ipinVDD, HIGH);
  pinMode(ipinVSS, OUTPUT); digitalWrite(ipinVSS, LOW);
  pinMode(ipin15,  OUTPUT); digitalWrite(ipin15,  LOW);
  pinMode(ipin16,  OUTPUT); digitalWrite(ipin16,  HIGH);

  for (uint8_t i = ipinDB7; i <= ipinDB4; i += 2) {
    pinMode(i, OUTPUT);
  }
  for (uint8_t i = ipinE; i <= ipinV0; i += 2) {
    pinMode(i, OUTPUT);
  }

  // Inicialização do LCD com ajuste de contraste PWM
  lcd.begin(16, 2);
  analogWrite(ipinV0, 64);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("  AULA 11_8 IA  "));
  lcd.setCursor(0, 1);
  lcd.print(F(" EDITOR NUMERICO"));
  vTaskDelay(pdMS_TO_TICKS(1200));
  lcd.clear();

  // Alimentação ativa do Encoder KY-040
  pinMode(ci_pinEnc_GND, OUTPUT); digitalWrite(ci_pinEnc_GND, LOW);
  pinMode(ci_pinEnc_VCC, OUTPUT); digitalWrite(ci_pinEnc_VCC, HIGH);
  pinMode(ci_pinEnc_CLK, INPUT_PULLUP);
  pinMode(ci_pinEnc_CCLK, INPUT_PULLUP);
  pinMode(ci_pinEnc_BTN, INPUT_PULLUP);

  // Criação da Fila de Eventos (16 mensagens)
  xEncoderQueue = xQueueCreate(16, sizeof(eEncoderEvent));

  // Criação das Tarefas com pilhas dimensionadas com segurança
  xTaskCreate(TaskEncoder,   "Encoder",   128, NULL, 3, NULL);
  xTaskCreate(TaskUI,        "UI",        256, NULL, 2, NULL);
  xTaskCreate(TaskHeartbeat, "Heartbeat", 128, NULL, 1, NULL);
}

void loop() {
  // FreeRTOS assume o controle do microcontrolador
}

// ==============================================================================
// TAREFA 1: DEBOUNCE E DECODIFICAÇÃO DO ENCODER KY-040 (PRODUTOR)
// ==============================================================================
void TaskEncoder(void *pvParameters) {
  (void) pvParameters;

  uint8_t u8_lastCLK = digitalRead(ci_pinEnc_CLK);
  uint32_t u32_btnPressTime = 0;
  bool b_btnPressed = false;
  bool b_longClickSent = false;

  for (;;) {
    const uint32_t u32_agora = millis();

    // 1. Decodificação de Rotação por Transição de Borda com Anti-Repique
    uint8_t u8_currentCLK = digitalRead(ci_pinEnc_CLK);
    if (u8_currentCLK != u8_lastCLK && u8_currentCLK == LOW) {
      eEncoderEvent event;
      if (digitalRead(ci_pinEnc_CCLK) != LOW) {
        event = ENC_EVENT_CW;  // Horário
      } else {
        event = ENC_EVENT_CCW; // Anti-horário
      }
      xQueueSend(xEncoderQueue, &event, 0);
    }
    u8_lastCLK = u8_currentCLK;

    // 2. Leitura Inteligente do Botão Central (Curto vs Longo)
    bool b_btnCurrentState = (digitalRead(ci_pinEnc_BTN) == LOW);

    if (b_btnCurrentState && !b_btnPressed) {
      // Início do clique
      b_btnPressed = true;
      b_longClickSent = false;
      u32_btnPressTime = u32_agora;
    } else if (b_btnCurrentState && b_btnPressed) {
      // Pressionamento contínuo: verifica se atingiu o limiar de clique longo (1.2s)
      if (!b_longClickSent && (u32_agora - u32_btnPressTime >= 1200UL)) {
        eEncoderEvent event = ENC_EVENT_LONG_CLICK;
        xQueueSend(xEncoderQueue, &event, 0);
        b_longClickSent = true;
      }
    } else if (!b_btnCurrentState && b_btnPressed) {
      // Botão liberado: se não foi clique longo e durou > 40ms, é clique simples
      if (!b_longClickSent && (u32_agora - u32_btnPressTime >= 40UL)) {
        eEncoderEvent event = ENC_EVENT_CLICK;
        xQueueSend(xEncoderQueue, &event, 0);
      }
      b_btnPressed = false;
      b_longClickSent = false;
    }

    vTaskDelay(pdMS_TO_TICKS(3)); // Amostragem rápida e leve a 3 ms
  }
}

// ==============================================================================
// FUNÇÕES AUXILIARES DE FORMATAÇÃO DO DISPLAY
// ==============================================================================
void formatarNumero6Digitos(uint32_t val, char *buffer) {
  // Preenche o buffer com zeros à esquerda: ex: 004500
  for (int8_t i = 5; i >= 0; i--) {
    buffer[i] = '0' + (val % 10);
    val /= 10;
  }
  buffer[6] = '\0';
}

void renderizarTela(uint32_t valor, uint8_t casa, eModoUI modo) {
  char strNum[7];
  formatarNumero6Digitos(valor, strNum);

  // Linha 0: "VAL: [ 000000 ] " (Dígitos começam na coluna 7)
  lcd.setCursor(0, 0);
  lcd.print(F("VAL: [ "));
  lcd.print(strNum);
  lcd.print(F(" ]"));

  // Linha 1: Status da interface e indicador da casa ativa
  lcd.setCursor(0, 1);
  if (modo == MODO_NAVEGACAO) {
    lcd.print(F("NAV "));
    // Monta a linha com o indicador '^' alinhado sob o dígito selecionado
    for (uint8_t c = 0; c < 6; c++) {
      if (c == casa) {
        lcd.print('^');
      } else {
        lcd.print(' ');
      }
    }
    lcd.print(F("  [SEL]"));
    lcd.noCursor();
    lcd.noBlink();
  } else {
    lcd.print(F("EDIT: [+/-]  [OK]"));
    // No modo edição, posiciona o cursor piscante exatamente no dígito ativo
    lcd.setCursor(7 + casa, 0);
    lcd.cursor();
    lcd.blink();
  }
}

// ==============================================================================
// TAREFA 2: MÁQUINA DE ESTADOS DA INTERFACE DO USUÁRIO (CONSUMIDOR)
// ==============================================================================
void TaskUI(void *pvParameters) {
  (void) pvParameters;

  uint32_t u32_valor = 0UL;
  uint8_t  u8_casa = 0; // 0 = 100.000 (esquerda) até 5 = 1 (direita)
  eModoUI  e_modo = MODO_NAVEGACAO;

  renderizarTela(u32_valor, u8_casa, e_modo);

  Serial.println(F("[IHM INICIADA] Modo: NAVEGACAO | Valor: 000000"));

  eEncoderEvent event;
  for (;;) {
    // Bloqueia com segurança aguardando eventos da fila
    if (xQueueReceive(xEncoderQueue, &event, portMAX_DELAY) == pdPASS) {

      // Reset rápido com clique longo (> 1.2s)
      if (event == ENC_EVENT_LONG_CLICK) {
        u32_valor = 0UL;
        u8_casa = 0;
        e_modo = MODO_NAVEGACAO;
        renderizarTela(u32_valor, u8_casa, e_modo);
        Serial.println(F("[RESET] Valor zerado para 000000"));
        continue;
      }

      // ----------------------------------------------------------------------
      // MODO 1: NAVEGAÇÃO DE CASAS DECIMAIS
      // ----------------------------------------------------------------------
      if (e_modo == MODO_NAVEGACAO) {
        if (event == ENC_EVENT_CW) {
          // Gira para Direita: avança para a próxima casa (trava no fim: casa 5)
          if (u8_casa < 5) {
            u8_casa++;
          }
        } else if (event == ENC_EVENT_CCW) {
          // Gira para Esquerda: recua para a casa anterior (trava no início: casa 0)
          if (u8_casa > 0) {
            u8_casa--;
          }
        } else if (event == ENC_EVENT_CLICK) {
          // Clique simples: entra no modo de edição da casa selecionada
          e_modo = MODO_EDICAO;
          Serial.print(F("[MODO EDICAO] Casa ativa: "));
          Serial.print(u8_casa);
          Serial.print(F(" (Peso: "));
          Serial.print(PESOS_DECIMAIS[u8_casa]);
          Serial.println(F(")"));
        }
      }
      // ----------------------------------------------------------------------
      // MODO 2: EDIÇÃO DA CASA SELECIONADA
      // ----------------------------------------------------------------------
      else if (e_modo == MODO_EDICAO) {
        const uint32_t peso = PESOS_DECIMAIS[u8_casa];

        if (event == ENC_EVENT_CW) {
          // Gira para Direita: soma o peso decimal sem ultrapassar o valor máximo
          if (u32_valor + peso <= VALOR_MAXIMO) {
            u32_valor += peso;
          } else {
            u32_valor = VALOR_MAXIMO; // Trava no limite máximo de 999999
          }
        } else if (event == ENC_EVENT_CCW) {
          // Gira para Esquerda: subtrai o peso decimal sem ficar negativo
          if (u32_valor >= peso) {
            u32_valor -= peso;
          } else {
            u32_valor = VALOR_MINIMO; // Trava no limite mínimo de 0
          }
        } else if (event == ENC_EVENT_CLICK) {
          // Clique simples: confirma e volta ao modo navegação
          e_modo = MODO_NAVEGACAO;
          Serial.print(F("[MODO NAVEGACAO] Valor Confirmado: "));
          Serial.println(u32_valor);
        }
      }

      // Atualiza visualmente o display LCD com o novo estado
      renderizarTela(u32_valor, u8_casa, e_modo);
    }
  }
}

// ==============================================================================
// TAREFA 3: HEARTBEAT DO FREERTOS (LED INDICADOR)
// ==============================================================================
void TaskHeartbeat(void *pvParameters) {
  (void) pvParameters;
  pinMode(LED_BUILTIN, OUTPUT);

  for (;;) {
    digitalWrite(LED_BUILTIN, HIGH);
    vTaskDelay(pdMS_TO_TICKS(1000));
    digitalWrite(LED_BUILTIN, LOW);
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}
