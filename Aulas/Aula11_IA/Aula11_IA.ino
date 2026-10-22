#include <Arduino_FreeRTOS.h>

void TaskBlink( void *pvParameters );
void TaskUART(void *pvParameters);
void TaskENCODERRead(void *pvParameters);

const int ci_pinEnc_GND = 8;
const int ci_pinEnc_VCC = 9;
const int ci_pinEnc_BTN = 10;
const int ci_pinEnc_CLK = 11;
const int ci_pinEnc_CCLK = 12;

typedef enum{
  EPRESSIONADO,
  ESOLTO,
} eBTState;

typedef enum{
  D,
  E,
  P,
} eCKState;

typedef struct{
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
  stBTNENC0.eBT_estadoAnt_CK = P;
  stBTNENC0.eBT_estadoAnt = ESOLTO;
  stBTNENC0.eBT_estadoAtual = ESOLTO;

  xTaskCreate(TaskBlink, "Blink", 128, NULL, 1, NULL);

  xTaskCreate(TaskUART, "TaskUART", 128, NULL, 1, NULL);

  xTaskCreate(TaskENCODERRead, "TaskENCODERRead", 128, NULL, 1, NULL);
}

void loop() {
  // put your main code here, to run repeatedly:
}

void TaskBlink(void *pvParameters){
  (void) pvParameters;
  pinMode(LED_BUILTIN, OUTPUT);

  for (;;)
  {
    digitalWrite(LED_BUILTIN, HIGH);
    vTaskDelay( 1000 / portTICK_PERIOD_MS );
    digitalWrite(LED_BUILTIN, LOW);
    vTaskDelay( 1000 / portTICK_PERIOD_MS );
    //Serial.println("Blink");
  }
}

void TaskUART(void *pvParameters){
  (void) pvParameters;
  for(;;){
    //int i_encoderPins=(!digitalRead(ci_pinEnc_CLK)<<2)|(!digitalRead(ci_pinEnc_CCLK)<<1)|(!digitalRead(ci_pinEnc_BTN)<<0);
    Serial.print("Estado: " + String(stBTNENC0.eBT_estadoAtual, HEX));
    Serial.print(", " + String(stBTNENC0.u32_Tempopressionado));
    Serial.print(", " + String(stBTNENC0.u32_TempopressionadoPosSolto));
    Serial.print(", CK: " + String(stBTNENC0.eBT_estadoAtual_CK, HEX));
    Serial.print(", " + String(stBTNENC0.u32_Tempopressionado_CK));
    Serial.print(", " + String(stBTNENC0.u32_TempopressionadoPosSolto_CK));
    Serial.print("\n");
    vTaskDelay(1);
  }
}

void TaskENCODERRead(void *pvParameters){
  (void) pvParameters;
  for(;;){
    if(!digitalRead(ci_pinEnc_BTN)){
      if(stBTNENC0.eBT_estadoAtual != EPRESSIONADO){
        stBTNENC0.u32_TempoAtual = millis();
      }
      stBTNENC0.u32_Tempopressionado = millis() - stBTNENC0.u32_TempoAtual;
      stBTNENC0.eBT_estadoAnt = stBTNENC0.eBT_estadoAtual;
      stBTNENC0.eBT_estadoAtual = EPRESSIONADO;
    }else{
      if(stBTNENC0.eBT_estadoAtual == EPRESSIONADO){
        stBTNENC0.u32_TempopressionadoPosSolto = millis() - stBTNENC0.u32_TempoAtual;
      }
      stBTNENC0.eBT_estadoAnt = stBTNENC0.eBT_estadoAtual;
      stBTNENC0.eBT_estadoAtual = ESOLTO;
    }

    if(stBTNENC0.eBT_estadoAtual_CK != P){
      /* Se passaram mais de 500 ms desde o último pulso registrado na ISR */
      if((millis() - stBTNENC0.u32_TempoAtual_CK) > (stBTNENC0.u32_Tempopressionado_CK + 500UL)){
        stBTNENC0.u32_TempopressionadoPosSolto_CK = stBTNENC0.u32_Tempopressionado_CK;
        stBTNENC0.eBT_estadoAnt_CK = stBTNENC0.eBT_estadoAtual_CK;
        stBTNENC0.eBT_estadoAtual_CK = P;
      }
    }

    vTaskDelay(1);
  }
}

ISR(PCINT0_vect){
  bool b_clk  = digitalRead(ci_pinEnc_CLK);
  bool b_cclk = digitalRead(ci_pinEnc_CCLK);

  if(stBTNENC0.eBT_estadoAtual_CK == P){
    if(!b_cclk && b_clk){
      //Serial.println("D");
      stBTNENC0.u32_TempoAtual_CK = millis();
      stBTNENC0.u32_Tempopressionado_CK = 0;
      stBTNENC0.eBT_estadoAnt_CK = stBTNENC0.eBT_estadoAtual_CK;
      stBTNENC0.eBT_estadoAtual_CK = D;
    }
    else if(!b_clk && b_cclk){
      //Serial.println("E");
      stBTNENC0.u32_TempoAtual_CK = millis();
      stBTNENC0.u32_Tempopressionado_CK = 0;
      stBTNENC0.eBT_estadoAnt_CK = stBTNENC0.eBT_estadoAtual_CK;
      stBTNENC0.eBT_estadoAtual_CK = E;
    }
  }
  else{
    /* Atualiza o tempo acumulado de giro a cada novo pulso da quadratura */
    if((!b_cclk && b_clk) || (!b_clk && b_cclk)){
      stBTNENC0.u32_Tempopressionado_CK = millis() - stBTNENC0.u32_TempoAtual_CK;
    }
  }
}
