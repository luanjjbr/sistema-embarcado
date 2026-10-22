#include <Arduino_FreeRTOS.h>
#include <LiquidCrystal.h>

// Protótipos das tarefas FreeRTOS
void TaskBlink(void *pvParameters);
void TaskUART(void *pvParameters);
void TaskENCODERRead(void *pvParameters);
void TaskDisplay(void *pvParameters);

// ==============================================================================
// CONFIGURAÇÃO DE PINAGEM CONFORME ESQUEMÁTICO REAL DO HARDWARE (PEC11R)
// ==============================================================================
// O esquemático possui resistores físicos de pull-up R42, R43 e R44 de 10k ligados a +5V:
const int ci_pinEnc_SW  = 8;  // Pino 8  (PH5)          - Botão Central (SW)
const int ci_pinEnc_DT  = 9;  // Pino 9  (PH6)          - Canal B (DT)
const int ci_pinEnc_CLK = 10; // Pino 10 (PB4 / PCINT4) - Canal A (CLK)

typedef enum {
  EPRESSIONADO,
  ESOLTO,
} eBTState;

typedef enum {
  D, // Direita (Horário)
  E, // Esquerda (Anti-horário)
  P, // Parado (Repouso)
} eCKState;

typedef struct {
  uint32_t u32_TempoAtual;
  uint32_t u32_TempoAtual_CK;
  uint32_t u32_TempoAtual_CK_1;
  uint32_t u32_TempopressionadoPosSolto;
  uint32_t u32_Tempopressionado;
  uint32_t u32_TempopressionadoPosSolto_CK;
  uint32_t u32_Tempopressionado_CK;
  eBTState eBT_estadoAtual;
  eBTState eBT_estadoAnt;
  eCKState eBT_estadoAtual_CK;
  eCKState eBT_estadoAnt_CK;
} stBTNENC;

volatile stBTNENC stBTNENC0;
volatile int32_t Valor = 0; // Acumulador exibido no display LCD

// ==============================================================================
// SETUP E INICIALIZAÇÃO
// ==============================================================================
void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ;
  }

  // Pinos do Encoder com pull-up habilitado
  pinMode(ci_pinEnc_SW,  INPUT_PULLUP);
  pinMode(ci_pinEnc_DT,  INPUT_PULLUP);
  pinMode(ci_pinEnc_CLK, INPUT_PULLUP);

  // Configuração dos Registradores de Interrupção no ATmega2560:
  // Como apenas o Pino 10 (PB4) tem PCINT, habilitamos EXCLUSIVAMENTE o PCINT4:
  PCICR  |= (1 << PCIE0);  // Habilita interrupção do bloco PORTB
  PCMSK0  = (1 << PCINT4); // 0b00010000 -> Somente o Pino 10 (CLK) gera interrupção!

  stBTNENC0.eBT_estadoAtual_CK = P;
  stBTNENC0.eBT_estadoAnt_CK   = P;
  stBTNENC0.eBT_estadoAnt      = ESOLTO;
  stBTNENC0.eBT_estadoAtual    = ESOLTO;

  // Criação das Tarefas FreeRTOS
  xTaskCreate(TaskDisplay,     "Display",     256, NULL, 1, NULL);
  xTaskCreate(TaskBlink,       "Blink",       128, NULL, 1, NULL);
  xTaskCreate(TaskUART,        "TaskUART",    256, NULL, 1, NULL);
  xTaskCreate(TaskENCODERRead, "TaskENCODERRead", 128, NULL, 1, NULL);
}

void loop() {
  // FreeRTOS assume o controle do microcontrolador
}

// ==============================================================================
// INTERRUPÇÃO POR MUDANÇA DE PINO (PCINT0) - ATIVAÇÃO NATIVA NA DESCIDA
// ==============================================================================
ISR(PCINT0_vect) {
  // Como apenas o Pino 10 (CLK) está habilitado na máscara PCMSK0,
  // a interrupção só acorda por transições deste pino.
  // Se o pino 10 estiver LOW, trata-se ESTRITAMENTE da BORDA DE DESCIDA (1 -> 0):
  if (digitalRead(ci_pinEnc_CLK) == LOW) {
    if (digitalRead(ci_pinEnc_DT) == HIGH) {
      Valor++;
      stBTNENC0.eBT_estadoAnt_CK = stBTNENC0.eBT_estadoAtual_CK;
      stBTNENC0.eBT_estadoAtual_CK = D; // Sentido Horário (Direita)
    } else {
      Valor--;
      stBTNENC0.eBT_estadoAnt_CK = stBTNENC0.eBT_estadoAtual_CK;
      stBTNENC0.eBT_estadoAtual_CK = E; // Sentido Anti-horário (Esquerda)
    }
    stBTNENC0.u32_TempoAtual_CK = millis();
    stBTNENC0.u32_TempoAtual_CK_1 = millis();
  }
}

// ==============================================================================
// TAREFA 1: BLINK (INDICADOR DE VIDA DO FREERTOS)
// ==============================================================================
void TaskBlink(void *pvParameters) {
  (void) pvParameters;
  pinMode(LED_BUILTIN, OUTPUT);

  for (;;) {
    digitalWrite(LED_BUILTIN, HIGH);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    digitalWrite(LED_BUILTIN, LOW);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

// ==============================================================================
// TAREFA 2: TELEMETRIA UART (SERIAL MONITOR)
// ==============================================================================
void TaskUART(void *pvParameters) {
  (void) pvParameters;
  for (;;) {
    Serial.print(F("SW: "));
    Serial.print(stBTNENC0.eBT_estadoAtual == EPRESSIONADO ? F("PRESS") : F("SOLTO"));
    Serial.print(F(" | CK: "));
    if (stBTNENC0.eBT_estadoAtual_CK == D) {
      Serial.print(F("DIR"));
    } else if (stBTNENC0.eBT_estadoAtual_CK == E) {
      Serial.print(F("ESQ"));
    } else {
      Serial.print(F("PAR"));
    }
    Serial.print(F(" | Valor: "));
    Serial.println(Valor);

    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

// ==============================================================================
// TAREFA 3: LEITURA E DEBOUNCE DO BOTÃO SW (PINO 8) E RESTAURAÇÃO DE ESTADO
// ==============================================================================
void TaskENCODERRead(void *pvParameters) {
  (void) pvParameters;
  for (;;) {
    // Leitura do botão SW no Pino 8
    if (!digitalRead(ci_pinEnc_SW)) {
      if (stBTNENC0.eBT_estadoAtual != EPRESSIONADO) {
        stBTNENC0.u32_TempoAtual = millis();
      }
      stBTNENC0.u32_Tempopressionado = millis() - stBTNENC0.u32_TempoAtual;
      stBTNENC0.eBT_estadoAnt = stBTNENC0.eBT_estadoAtual;
      stBTNENC0.eBT_estadoAtual = EPRESSIONADO;
    } else {
      if (stBTNENC0.eBT_estadoAtual == EPRESSIONADO) {
        stBTNENC0.u32_TempopressionadoPosSolto = millis() - stBTNENC0.u32_TempoAtual;
      }
      stBTNENC0.eBT_estadoAnt = stBTNENC0.eBT_estadoAtual;
      stBTNENC0.eBT_estadoAtual = ESOLTO;
    }

    // Retorna automaticamente ao estado Parado (P) após 250ms de inatividade
    if (stBTNENC0.eBT_estadoAtual_CK != P) {
      stBTNENC0.u32_Tempopressionado_CK = millis() - stBTNENC0.u32_TempoAtual_CK_1;
      if (millis() > stBTNENC0.u32_TempoAtual_CK + (500 / portTICK_PERIOD_MS))
      {
        stBTNENC0.u32_TempopressionadoPosSolto_CK = stBTNENC0.u32_Tempopressionado_CK;
        stBTNENC0.eBT_estadoAnt_CK = stBTNENC0.eBT_estadoAtual_CK;
        stBTNENC0.eBT_estadoAtual_CK = P;
      }
    }
    vTaskDelay(1);
  }
}

// ==============================================================================
// TAREFA 4: DISPLAY LCD 16x2 HD44780
// ==============================================================================
void TaskDisplay(void *pvParameters) {
  (void) pvParameters;
  const int ipinDB7 = 23, ipinDB6 = 25, ipinDB5 = 27, ipinDB4 = 29;
  const int ipinE   = 39, ipinRW  = 41, ipinRS  = 43, ipinV0  = 45;
  const int ipinVDD = 47, ipinVSS = 49, ipin15  = 51, ipin16  = 53;

  pinMode(ipinVDD, OUTPUT); digitalWrite(ipinVDD, HIGH);
  pinMode(ipinVSS, OUTPUT); digitalWrite(ipinVSS, LOW);
  pinMode(ipin15,  OUTPUT); digitalWrite(ipin15,  LOW);
  pinMode(ipin16,  OUTPUT); digitalWrite(ipin16,  HIGH);

  for (uint16_t i = ipinDB7; i <= ipinDB4; i += 2) {
    pinMode(i, OUTPUT);
  }
  for (uint16_t i = ipinE; i <= ipinV0; i += 2) {
    pinMode(i, OUTPUT);
  }

  LiquidCrystal lcd(ipinRS, ipinRW, ipinE, ipinDB4, ipinDB5, ipinDB6, ipinDB7);
  lcd.begin(16, 2);
  lcd.print(F("AULA 11_8 HW"));
  lcd.setCursor(0, 1);
  lcd.print(F("PINOS D10 D9 D8"));
  analogWrite(ipinV0, 64);
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  lcd.clear();

  for (;;) {
    lcd.setCursor(0, 0);
    lcd.print(F("VALOR: "));
    lcd.print(Valor);
    lcd.print(F("        "));

    lcd.setCursor(0, 1);
    lcd.print(F("DIR: "));
    if (stBTNENC0.eBT_estadoAtual_CK == D) {
      lcd.print(F("DIREITA "));
    } else if (stBTNENC0.eBT_estadoAtual_CK == E) {
      lcd.print(F("ESQUERDA"));
    } else {
      lcd.print(F("PARADO  "));
    }
    lcd.print(F(" BT:"));
    lcd.print(stBTNENC0.eBT_estadoAtual == EPRESSIONADO ? F("1") : F("0"));

    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

