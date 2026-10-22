#include <Arduino_FreeRTOS.h>
#include <LiquidCrystal.h>

// Protótipos das tarefas
void TaskBlink(void *pvParameters);
void TaskUART(void *pvParameters);
void TaskENCODERRead(void *pvParameters);
void TaskDisplay(void *pvParameters);

// Pinos do Encoder KY-040 (vindos da Aula 11)
const int ci_pinEnc_GND = 8;
const int ci_pinEnc_VCC = 9;
const int ci_pinEnc_BTN = 10;
const int ci_pinEnc_CLK = 11;
const int ci_pinEnc_CCLK = 12;

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
volatile int32_t Valor = 0; // Acumulador do encoder exibido no display da Aula 8

void setup() {
  PCICR  |= 0b00000001;
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

  // Criação das tarefas: Display (Aula 8) + Blink, UART e Encoder (Aula 11)
  xTaskCreate(TaskDisplay,     "Display",     256, NULL, 1, NULL);
  xTaskCreate(TaskBlink,       "Blink",       128, NULL, 1, NULL);
  xTaskCreate(TaskUART,        "TaskUART",    256, NULL, 1, NULL);
  xTaskCreate(TaskENCODERRead, "TaskENCODERRead", 128, NULL, 1, NULL);
}

void loop() {
  // FreeRTOS assume o agendamento
}

// Tarefa Blink (vinda da Aula 11)
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

// Tarefa UART (vinda da Aula 11 - sem String, estável no SimulIDE)
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

// Tarefa Leitura do Encoder (vinda da Aula 11)
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
      if (millis() > stBTNENC0.u32_TempoAtual_CK + (5000 / portTICK_PERIOD_MS)) {
        stBTNENC0.u32_TempopressionadoPosSolto_CK = stBTNENC0.u32_Tempopressionado_CK;
        stBTNENC0.eBT_estadoAnt_CK = stBTNENC0.eBT_estadoAtual_CK;
        stBTNENC0.eBT_estadoAtual_CK = P;
      }
    }
    vTaskDelay(1);
  }
}

// Interrupção por Mudança de Pino (vinda da Aula 11)
ISR(PCINT0_vect) {
  if (!digitalRead(ci_pinEnc_CCLK) && digitalRead(ci_pinEnc_CLK) && stBTNENC0.eBT_estadoAtual_CK == P) {
    stBTNENC0.u32_TempoAtual_CK = millis();
    stBTNENC0.u32_TempoAtual_CK_1 = millis();
    stBTNENC0.eBT_estadoAnt_CK = stBTNENC0.eBT_estadoAtual_CK;
    stBTNENC0.eBT_estadoAtual_CK = D;
  }
  if (!digitalRead(ci_pinEnc_CLK) && digitalRead(ci_pinEnc_CCLK) && stBTNENC0.eBT_estadoAtual_CK == P) {
    stBTNENC0.u32_TempoAtual_CK = millis();
    stBTNENC0.u32_TempoAtual_CK_1 = millis();
    stBTNENC0.eBT_estadoAnt_CK = stBTNENC0.eBT_estadoAtual_CK;
    stBTNENC0.eBT_estadoAtual_CK = E;
  }

  if (!digitalRead(ci_pinEnc_CCLK) && digitalRead(ci_pinEnc_CLK) && stBTNENC0.eBT_estadoAtual_CK != P) {
    stBTNENC0.u32_TempoAtual_CK = millis();
  } else if (!digitalRead(ci_pinEnc_CLK) && digitalRead(ci_pinEnc_CCLK) && stBTNENC0.eBT_estadoAtual_CK != P) {
    stBTNENC0.u32_TempoAtual_CK = millis();
  }
}

// Tarefa Display LCD 16x2 (vinda da Aula 8)
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
  lcd.print("hello, world1!");
  lcd.setCursor(0, 1);
  lcd.print("hello, world2!");
  analogWrite(ipinV0, 64);
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  lcd.clear();

  for (;;) {
    // Atualiza a variável Valor a partir do estado do encoder
    if (stBTNENC0.eBT_estadoAtual_CK == D) {
      Valor += 1;
    }
    if (stBTNENC0.eBT_estadoAtual_CK == E) {
      Valor -= 1;
    }

    lcd.setCursor(0, 0);
    lcd.print("A0:");
    lcd.setCursor(0, 1);
    lcd.print(Valor);
    lcd.print("        "); // Limpa caracteres remanescentes

    vTaskDelay(200 / portTICK_PERIOD_MS);
  }
}
