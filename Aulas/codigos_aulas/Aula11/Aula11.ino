#include <Arduino_FreeRTOS.h>

void TaskBlink(void *pvParameters);
void TaskUART(void *pvParameters);
void TaskENCODERRead(void *pvParameters);

// Pinos do Encoder KY-040 (padrão de bancada D8, D9, D10)
const int ci_pinEnc_BTN  = 8;  // SW (Pino 8)
const int ci_pinEnc_CCLK = 9;  // DT (Pino 9)
const int ci_pinEnc_CLK  = 10; // CLK (Pino 10 / PCINT4)

typedef enum {
  EPRESSIONADO,
  ESOLTO,
} eBTState;

typedef enum {
  D,
  E,
  P,
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

void setup() {
  // Habilita interrupção PCINT exclusivamente para o Pino 10 (PCINT4)
  PCICR  |= (1 << PCIE0);
  PCMSK0  = (1 << PCINT4);

  Serial.begin(9600);
  while (!Serial) {
    ;
  }

  pinMode(ci_pinEnc_CLK, INPUT_PULLUP);
  pinMode(ci_pinEnc_CCLK, INPUT_PULLUP);
  pinMode(ci_pinEnc_BTN, INPUT_PULLUP);

  stBTNENC0.eBT_estadoAtual_CK = P;
  stBTNENC0.eBT_estadoAnt_CK   = P;
  stBTNENC0.eBT_estadoAnt      = ESOLTO;
  stBTNENC0.eBT_estadoAtual    = ESOLTO;

  xTaskCreate(TaskBlink,       "Blink",       128, NULL, 1, NULL);
  xTaskCreate(TaskUART,        "TaskUART",    256, NULL, 1, NULL);
  xTaskCreate(TaskENCODERRead, "TaskENCODERRead", 128, NULL, 1, NULL);
}

void loop() {
  // FreeRTOS assume o controle do microcontrolador
}

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

void TaskUART(void *pvParameters) {
  (void) pvParameters;
  for (;;) {
    Serial.print(F("Estado: "));
    Serial.print(stBTNENC0.eBT_estadoAtual);
    Serial.print(F(", "));
    Serial.print(stBTNENC0.u32_Tempopressionado);
    Serial.print(F(", "));
    Serial.print(stBTNENC0.u32_TempopressionadoPosSolto);
    Serial.print(F(", CK: "));
    Serial.print(stBTNENC0.eBT_estadoAtual_CK);
    Serial.print(F(", "));
    Serial.print(stBTNENC0.u32_Tempopressionado_CK);
    Serial.print(F(", "));
    Serial.print(stBTNENC0.u32_TempopressionadoPosSolto_CK);
    Serial.println();
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void TaskENCODERRead(void *pvParameters) {
  (void) pvParameters;
  for (;;) {
    if (!digitalRead(ci_pinEnc_BTN)) {
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

    if (stBTNENC0.eBT_estadoAtual_CK != P) {
      stBTNENC0.u32_Tempopressionado_CK = millis() - stBTNENC0.u32_TempoAtual_CK_1;
      if (millis() > stBTNENC0.u32_TempoAtual_CK + (500 / portTICK_PERIOD_MS)) {
        stBTNENC0.u32_TempopressionadoPosSolto_CK = stBTNENC0.u32_Tempopressionado_CK;
        stBTNENC0.eBT_estadoAnt_CK = stBTNENC0.eBT_estadoAtual_CK;
        stBTNENC0.eBT_estadoAtual_CK = P;
      }
    }
    vTaskDelay(1);
  }
}

ISR(PCINT0_vect) {
  // Como apenas o Pino 10 acorda o vetor, se estiver LOW é Borda de Descida:
  if (digitalRead(ci_pinEnc_CLK) == LOW) {
    if (digitalRead(ci_pinEnc_CCLK) == HIGH) {
      stBTNENC0.u32_TempoAtual_CK = millis();
      stBTNENC0.u32_TempoAtual_CK_1 = millis();
      stBTNENC0.eBT_estadoAnt_CK = stBTNENC0.eBT_estadoAtual_CK;
      stBTNENC0.eBT_estadoAtual_CK = D;
    } else {
      stBTNENC0.u32_TempoAtual_CK = millis();
      stBTNENC0.u32_TempoAtual_CK_1 = millis();
      stBTNENC0.eBT_estadoAnt_CK = stBTNENC0.eBT_estadoAtual_CK;
      stBTNENC0.eBT_estadoAtual_CK = E;
    }
  }
}
