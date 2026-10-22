#include <Arduino_FreeRTOS.h>

void TaskBlink(void *pvParameters);
void TaskUART(void *pvParameters);
void TaskENCODERRead(void *pvParameters);

const uint8_t ci_pinEnc_GND  = 8;
const uint8_t ci_pinEnc_VCC  = 9;
const uint8_t ci_pinEnc_BTN  = 10;
const uint8_t ci_pinEnc_CLK  = 11;
const uint8_t ci_pinEnc_CCLK = 12;

typedef enum : uint8_t {
  EPRESSIONADO,
  ESOLTO,
} eBTState;

typedef enum : uint8_t {
  D,
  E,
  P,
} eCKState;

typedef struct {
  volatile uint32_t u32_TempoAtual;
  volatile uint32_t u32_TempoAtual_CK;
  volatile uint32_t u32_TempopressionadoPosSolto;
  volatile uint32_t u32_Tempopressionado;
  volatile uint32_t u32_TempopressionadoPosSolto_CK;
  volatile uint32_t u32_Tempopressionado_CK;
  volatile eBTState eBT_estadoAtual;
  volatile eBTState eBT_estadoAnt;
  volatile eCKState eBT_estadoAtual_CK;
  volatile eCKState eBT_estadoAnt_CK;
} stBTNENC;

stBTNENC stBTNENC0;

void setup() {
  /* Habilita interrupção Pin Change no grupo PCIE0 (Port B) */
  PCICR |= 0b00000001;
  /* Habilita PCINT4 (Pino 10), PCINT5 (Pino 11) e PCINT6 (Pino 12) */
  PCMSK0 |= 0b01110000;

  Serial.begin(9600);
  while (!Serial) {
    ;
  }

  pinMode(ci_pinEnc_GND, OUTPUT); digitalWrite(ci_pinEnc_GND, LOW);
  pinMode(ci_pinEnc_VCC, OUTPUT); digitalWrite(ci_pinEnc_VCC, HIGH);
  pinMode(ci_pinEnc_CLK, INPUT);
  pinMode(ci_pinEnc_CCLK, INPUT);
  pinMode(ci_pinEnc_BTN, INPUT);

  stBTNENC0.eBT_estadoAtual_CK = P;
  stBTNENC0.eBT_estadoAnt_CK   = P;
  stBTNENC0.eBT_estadoAnt      = ESOLTO;
  stBTNENC0.eBT_estadoAtual    = ESOLTO;

  xTaskCreate(TaskBlink,       "Blink",           128, NULL, 1, NULL);
  xTaskCreate(TaskUART,        "TaskUART",        128, NULL, 1, NULL);
  xTaskCreate(TaskENCODERRead, "TaskENCODERRead", 128, NULL, 1, NULL);
}

void loop() {
  // Vazio: execução gerenciada pelo escalonador do FreeRTOS
}

void TaskBlink(void *pvParameters) {
  (void) pvParameters;
  pinMode(LED_BUILTIN, OUTPUT);

  for (;;) {
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void TaskUART(void *pvParameters) {
  (void) pvParameters;
  for (;;) {
    Serial.print(F("Estado: "));
    Serial.print(stBTNENC0.eBT_estadoAtual, HEX);
    Serial.print(F(", "));
    Serial.print(stBTNENC0.u32_Tempopressionado);
    Serial.print(F(", "));
    Serial.print(stBTNENC0.u32_TempopressionadoPosSolto);
    Serial.print(F(", CK: "));
    Serial.print(stBTNENC0.eBT_estadoAtual_CK, HEX);
    Serial.print(F(", "));
    Serial.print(stBTNENC0.u32_Tempopressionado_CK);
    Serial.print(F(", "));
    Serial.println(stBTNENC0.u32_TempopressionadoPosSolto_CK);

    vTaskDelay(1);
  }
}

void TaskENCODERRead(void *pvParameters) {
  (void) pvParameters;
  for (;;) {
    const uint32_t u32_agora = millis();

    if (!digitalRead(ci_pinEnc_BTN)) {
      if (stBTNENC0.eBT_estadoAtual != EPRESSIONADO) {
        stBTNENC0.u32_TempoAtual = u32_agora;
      }
      stBTNENC0.u32_Tempopressionado = u32_agora - stBTNENC0.u32_TempoAtual;
      stBTNENC0.eBT_estadoAnt        = stBTNENC0.eBT_estadoAtual;
      stBTNENC0.eBT_estadoAtual      = EPRESSIONADO;
    } else {
      if (stBTNENC0.eBT_estadoAtual == EPRESSIONADO) {
        stBTNENC0.u32_TempopressionadoPosSolto = u32_agora - stBTNENC0.u32_TempoAtual;
      }
      stBTNENC0.eBT_estadoAnt   = stBTNENC0.eBT_estadoAtual;
      stBTNENC0.eBT_estadoAtual = ESOLTO;
    }

    if (stBTNENC0.eBT_estadoAtual_CK != P) {
      /* Atualiza o tempo rodando quando entra na Task */
      stBTNENC0.u32_Tempopressionado_CK = u32_agora - stBTNENC0.u32_TempoAtual_CK;

      /* Quando para de rodar (> 500 ms sem pulso na ISR), atualiza o PosSolto e volta para P */
      if (stBTNENC0.u32_Tempopressionado_CK > 500UL) {
        stBTNENC0.u32_TempopressionadoPosSolto_CK = stBTNENC0.u32_Tempopressionado_CK;
        stBTNENC0.eBT_estadoAnt_CK                = stBTNENC0.eBT_estadoAtual_CK;
        stBTNENC0.eBT_estadoAtual_CK              = P;
      }
    }

    vTaskDelay(1);
  }
}

ISR(PCINT0_vect) {
  const bool b_clk  = digitalRead(ci_pinEnc_CLK);
  const bool b_cclk = digitalRead(ci_pinEnc_CCLK);

  if (b_clk != b_cclk) {
    stBTNENC0.u32_TempoAtual_CK = millis();
    if (stBTNENC0.eBT_estadoAtual_CK == P) {
      stBTNENC0.eBT_estadoAnt_CK   = P;
      stBTNENC0.eBT_estadoAtual_CK = b_clk ? D : E;
    }
  }
}
