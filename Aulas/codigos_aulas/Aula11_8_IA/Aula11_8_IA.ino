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
const uint8_t ci_pinEnc_BTN  = 10; // SW (Botão de clique - PCINT4 / PB4)
const uint8_t ci_pinEnc_CLK  = 11; // Canal A (CLK - PCINT5 / PB5)
const uint8_t ci_pinEnc_CCLK = 12; // Canal B (DT  - PCINT6 / PB6)

// Objeto de controle do display LCD
LiquidCrystal lcd(ipinRS, ipinRW, ipinE, ipinDB4, ipinDB5, ipinDB6, ipinDB7);

// ==============================================================================
// TIPOS E DEFINIÇÕES DO SISTEMA
// ==============================================================================
typedef enum {
  ENC_EVENT_NONE,
  ENC_EVENT_CW,         // Giro Horário (Direita)
  ENC_EVENT_CCW,        // Giro Anti-horário (Esquerda)
  ENC_EVENT_CLICK,      // Clique Curto (< 1.2s - Seleciona / Desseleciona)
  ENC_EVENT_LONG_CLICK  // Clique Longo (> 1.2s - Reset para 000000)
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

// Fila FreeRTOS para transporte seguro de eventos do Encoder/Botão para a UI
QueueHandle_t xEncoderQueue = NULL;

// Estado anterior de CLK para detecção de borda no ISR
volatile uint8_t v_lastCLK = HIGH;

// Protótipos das Tarefas FreeRTOS
void TaskButton(void *pvParameters);
void TaskUI(void *pvParameters);
void TaskHeartbeat(void *pvParameters);

// ==============================================================================
// INTERRUPÇÃO POR MUDANÇA DE PINO (PCINT0) - QUADRATURA KY-040
// ==============================================================================
ISR(PCINT0_vect) {
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  uint8_t currentCLK = digitalRead(ci_pinEnc_CLK);
  uint8_t currentDT  = digitalRead(ci_pinEnc_CCLK);

  // Transição de descida no sinal CLK (Detent do encoder)
  if (v_lastCLK == HIGH && currentCLK == LOW) {
    eEncoderEvent event;
    if (currentDT == HIGH) {
      event = ENC_EVENT_CW;   // Horário (Direita)
    } else {
      event = ENC_EVENT_CCW;  // Anti-horário (Esquerda)
    }
    xQueueSendFromISR(xEncoderQueue, &event, &xHigherPriorityTaskWoken);
  }
  v_lastCLK = currentCLK;

  if (xHigherPriorityTaskWoken) {
    portYIELD_FROM_ISR();
  }
}

// ==============================================================================
// SETUP E INICIALIZAÇÃO
// ==============================================================================
void setup() {
  Serial.begin(9600);

  // Alimentação ativa dos trilhos do LCD (compatibilidade com hardware da Aula 8)
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

  // Alimentação ativa do Encoder KY-040 (compatibilidade com Aula 11)
  pinMode(ci_pinEnc_GND, OUTPUT); digitalWrite(ci_pinEnc_GND, LOW);
  pinMode(ci_pinEnc_VCC, OUTPUT); digitalWrite(ci_pinEnc_VCC, HIGH);
  pinMode(ci_pinEnc_CLK, INPUT_PULLUP);
  pinMode(ci_pinEnc_CCLK, INPUT_PULLUP);
  pinMode(ci_pinEnc_BTN, INPUT_PULLUP);

  v_lastCLK = digitalRead(ci_pinEnc_CLK);

  // Configuração da interrupção por mudança de pino (PCINT0) no ATmega2560
  PCICR  |= 0b00000001; // Habilita interrupção PCINT[7:0]
  PCMSK0 |= 0b01110000; // Habilita máscara para pinos 10 (PB4), 11 (PB5) e 12 (PB6)

  // Criação da Fila FreeRTOS (16 eventos)
  xEncoderQueue = xQueueCreate(16, sizeof(eEncoderEvent));

  // Criação das Tarefas com pilhas seguras
  xTaskCreate(TaskButton,    "Btn",       128, NULL, 2, NULL);
  xTaskCreate(TaskUI,        "UI",        256, NULL, 2, NULL);
  xTaskCreate(TaskHeartbeat, "Heartbeat", 128, NULL, 1, NULL);

  // NOTA CRÍTICA: Nenhuma chamada a vTaskDelay() pode ocorrer aqui no setup().
  // O escalonador FreeRTOS assume o controle automaticamente ao sair do setup().
}

void loop() {
  // Loop vazio: FreeRTOS gerencia o microcontrolador
}

// ==============================================================================
// TAREFA 1: DEBOUNCE E TEMPORIZAÇÃO DO BOTÃO CENTRAL (PRODUTOR)
// ==============================================================================
void TaskButton(void *pvParameters) {
  (void) pvParameters;

  uint32_t u32_pressStart = 0;
  bool b_isPressed = false;
  bool b_longReported = false;

  for (;;) {
    const uint32_t u32_agora = millis();
    bool b_currentPressed = (digitalRead(ci_pinEnc_BTN) == LOW);

    if (b_currentPressed && !b_isPressed) {
      // Borda de descida (botão pressionado)
      b_isPressed = true;
      b_longReported = false;
      u32_pressStart = u32_agora;
    } else if (b_currentPressed && b_isPressed) {
      // Pressionamento mantido: verifica clique longo (> 1.2s)
      if (!b_longReported && (u32_agora - u32_pressStart >= 1200UL)) {
        eEncoderEvent event = ENC_EVENT_LONG_CLICK;
        xQueueSend(xEncoderQueue, &event, 0);
        b_longReported = true;
      }
    } else if (!b_currentPressed && b_isPressed) {
      // Borda de subida (botão liberado): se foi clique rápido (> 40ms)
      if (!b_longReported && (u32_agora - u32_pressStart >= 40UL)) {
        eEncoderEvent event = ENC_EVENT_CLICK;
        xQueueSend(xEncoderQueue, &event, 0);
      }
      b_isPressed = false;
      b_longReported = false;
    }

    vTaskDelay(pdMS_TO_TICKS(10)); // Varredura periódica a cada 10ms
  }
}

// ==============================================================================
// FUNÇÕES DE FORMATAÇÃO E RENDERIZAÇÃO NO LCD 16x2
// ==============================================================================
void formatarNumero6Digitos(uint32_t val, char *buffer) {
  for (int8_t i = 5; i >= 0; i--) {
    buffer[i] = '0' + (val % 10);
    val /= 10;
  }
  buffer[6] = '\0';
}

void renderizarTela(uint32_t valor, uint8_t casa, eModoUI modo) {
  char strNum[7];
  formatarNumero6Digitos(valor, strNum);

  // Linha 0 (16 chars): "VAL: [ 000000 ] "
  lcd.setCursor(0, 0);
  lcd.print(F("VAL: [ "));
  lcd.print(strNum);
  lcd.print(F(" ] "));

  // Linha 1 (16 chars)
  lcd.setCursor(0, 1);
  if (modo == MODO_NAVEGACAO) {
    lcd.print(F("NAV: [ "));
    for (uint8_t c = 0; c < 6; c++) {
      if (c == casa) {
        lcd.print('^');
      } else {
        lcd.print(' ');
      }
    }
    lcd.print(F(" ] "));
    lcd.noCursor();
    lcd.noBlink();
  } else {
    lcd.print(F("EDT: [+/-]  [OK]"));
    // Posiciona o cursor de hardware exatamente sobre o dígito ativo (coluna 7 + casa)
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

  // Inicialização segura do display dentro do contexto de tarefa
  lcd.begin(16, 2);
  analogWrite(ipinV0, 64); // Contraste ótimo
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("  AULA 11_8 IA  "));
  lcd.setCursor(0, 1);
  lcd.print(F(" EDITOR NUMERICO"));
  
  // Atraso de apresentação com escalonador em execução
  vTaskDelay(pdMS_TO_TICKS(1200));
  lcd.clear();

  uint32_t u32_valor = 0UL;
  uint8_t  u8_casa = 0; // 0 = mais significativo (100k) até 5 = menos significativo (1)
  eModoUI  e_modo = MODO_NAVEGACAO;

  renderizarTela(u32_valor, u8_casa, e_modo);
  Serial.println(F("[IHM INICIADA] Modo: NAVEGACAO | Valor: 000000"));

  eEncoderEvent event;
  for (;;) {
    // Bloqueia com consumo zero de CPU até chegar evento do encoder ou botão
    if (xQueueReceive(xEncoderQueue, &event, portMAX_DELAY) == pdPASS) {

      // Reset total com clique longo (> 1.2s)
      if (event == ENC_EVENT_LONG_CLICK) {
        u32_valor = 0UL;
        u8_casa = 0;
        e_modo = MODO_NAVEGACAO;
        renderizarTela(u32_valor, u8_casa, e_modo);
        Serial.println(F("[RESET] Valor zerado para 000000"));
        continue;
      }

      // ----------------------------------------------------------------------
      // MODO 1: NAVEGAÇÃO ENTRE CASAS DECIMAIS (0 a 5)
      // ----------------------------------------------------------------------
      if (e_modo == MODO_NAVEGACAO) {
        if (event == ENC_EVENT_CW) {
          // Direita: avança para a próxima casa (trava no fim: casa 5)
          if (u8_casa < 5) {
            u8_casa++;
          }
        } else if (event == ENC_EVENT_CCW) {
          // Esquerda: recua para a casa anterior (trava no início: casa 0)
          if (u8_casa > 0) {
            u8_casa--;
          }
        } else if (event == ENC_EVENT_CLICK) {
          // Clique simples: seleciona e entra no modo edição
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
          // Direita: soma o peso sem estourar o valor máximo (999999)
          if (u32_valor + peso <= VALOR_MAXIMO) {
            u32_valor += peso;
          } else {
            u32_valor = VALOR_MAXIMO;
          }
        } else if (event == ENC_EVENT_CCW) {
          // Esquerda: subtrai o peso sem descer abaixo de zero
          if (u32_valor >= peso) {
            u32_valor -= peso;
          } else {
            u32_valor = VALOR_MINIMO;
          }
        } else if (event == ENC_EVENT_CLICK) {
          // Clique simples: desseleciona (confirma) e volta à navegação
          e_modo = MODO_NAVEGACAO;
          Serial.print(F("[MODO NAVEGACAO] Valor Confirmado: "));
          Serial.println(u32_valor);
        }
      }

      // Atualiza o display com o novo estado
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
