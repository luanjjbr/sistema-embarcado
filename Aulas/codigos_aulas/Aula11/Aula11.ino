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

typedef  struct{
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
  PCICR |=0b0000001;
  PCMSK0 |=0b01110000;
  

  Serial.begin(9600);

  while (!Serial) {
    ;
  }
  pinMode(ci_pinEnc_GND,OUTPUT);digitalWrite(ci_pinEnc_GND, LOW);
  pinMode(ci_pinEnc_VCC,OUTPUT);digitalWrite(ci_pinEnc_VCC, HIGH);
  pinMode(ci_pinEnc_CLK,INPUT);
  pinMode(ci_pinEnc_CCLK,INPUT);
  pinMode(ci_pinEnc_BTN,INPUT);

  stBTNENC0.eBT_estadoAtual_CK = P;
  stBTNENC0.eBT_estadoAnt_CK = P;
  stBTNENC0.eBT_estadoAnt = ESOLTO;
  stBTNENC0.eBT_estadoAtual = ESOLTO;
  
  xTaskCreate(TaskBlink,"Blink",128,NULL,1,NULL);

  xTaskCreate(TaskUART, "TaskUART", 256, NULL, 1, NULL);

  xTaskCreate(TaskENCODERRead,"TaskENCODERRead",128,NULL,1,NULL);

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
  for(;;){
    //int i_encoderPins=(!digitalRead(ci_pinEnc_CLK)<<2)|(!digitalRead(ci_pinEnc_CCLK)<<1)|(!digitalRead(ci_pinEnc_BTN)<<0);
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
    vTaskDelay( 100 / portTICK_PERIOD_MS );
  }
}
void TaskENCODERRead(void *pvParameters){
  for(;;){
    if(!digitalRead(ci_pinEnc_BTN)){
      if(stBTNENC0.eBT_estadoAtual != EPRESSIONADO ){
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

      stBTNENC0.u32_Tempopressionado_CK = millis() - stBTNENC0.u32_TempoAtual_CK_1;
      if((millis()>stBTNENC0.u32_TempoAtual_CK+(5000 / portTICK_PERIOD_MS))){
        stBTNENC0.u32_TempopressionadoPosSolto_CK = stBTNENC0.u32_Tempopressionado_CK;
        stBTNENC0.eBT_estadoAnt_CK = stBTNENC0.eBT_estadoAtual_CK;
        stBTNENC0.eBT_estadoAtual_CK = P;
      }
    }
    vTaskDelay(1);
  }
}
ISR(PCINT0_vect){
  if(!digitalRead(ci_pinEnc_CCLK) && digitalRead(ci_pinEnc_CLK) && stBTNENC0.eBT_estadoAtual_CK == P){
    //Serial.println("D");
    stBTNENC0.u32_TempoAtual_CK = millis();
    stBTNENC0.u32_TempoAtual_CK_1 = millis();
    stBTNENC0.eBT_estadoAnt_CK = stBTNENC0.eBT_estadoAtual_CK;
    stBTNENC0.eBT_estadoAtual_CK = D;
  }
  if(!digitalRead(ci_pinEnc_CLK) && digitalRead(ci_pinEnc_CCLK) && stBTNENC0.eBT_estadoAtual_CK == P){
    //Serial.println("E");
    stBTNENC0.u32_TempoAtual_CK = millis();
    stBTNENC0.u32_TempoAtual_CK_1 = millis();
    stBTNENC0.eBT_estadoAnt_CK = stBTNENC0.eBT_estadoAtual_CK;
    stBTNENC0.eBT_estadoAtual_CK = E;
  }

  if(!digitalRead(ci_pinEnc_CCLK) && digitalRead(ci_pinEnc_CLK) && stBTNENC0.eBT_estadoAtual_CK != P){
    stBTNENC0.u32_TempoAtual_CK = millis();
  }
  else if(!digitalRead(ci_pinEnc_CLK) && digitalRead(ci_pinEnc_CCLK) && stBTNENC0.eBT_estadoAtual_CK != P){
    stBTNENC0.u32_TempoAtual_CK = millis();
  }
}