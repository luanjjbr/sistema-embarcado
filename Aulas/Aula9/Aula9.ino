#include <Arduino_FreeRTOS.h>
#include <LiquidCrystal.h>

// Definições de constantes e pinos
#define DEF_NUMAQUISITION 64

const int Ci_Pin12 = 12;
const int Ci_Pin11 = 11;

volatile int sensorValue = 0;
volatile int sensorValueA1 = 0;

// Declaração das tarefas do FreeRTOS
void TaskBlink( void *pvParameters );
void TaskAnalogRead( void *pvParameters );
void TaskDisplay( void *pvParameters );
void vNewTaskAnalogRead( void *pvParameters );

void setup() {
  // Inicialização da comunicação serial a 9600 bps
  Serial.begin(9600);
  pinMode(Ci_Pin12, OUTPUT); digitalWrite(Ci_Pin12, LOW);
  pinMode(Ci_Pin11, OUTPUT); digitalWrite(Ci_Pin11, LOW);

  while (!Serial) {
    ; // Aguarda conexão da porta serial (necessário para placas com USB nativo)
  }

  // Criação das tarefas com prioridades e pilhas dimensionadas com segurança
  xTaskCreate(TaskBlink,          "Blink",          128,  NULL,  1,  NULL );
  xTaskCreate(TaskAnalogRead,     "AnalogRead",     128,  NULL,  1,  NULL );
  xTaskCreate(TaskDisplay,        "Display",        256,  NULL,  1,  NULL );
  xTaskCreate(vNewTaskAnalogRead, "AnalogRead_new", 160,  NULL,  2,  NULL );
}

void loop() {
  // Vazio: o escalonador do FreeRTOS assume o controle da CPU
}

/*--------------------------------------------------*/
/*---------------------- Tasks ---------------------*/
/*--------------------------------------------------*/

void TaskBlink(void *pvParameters)
{
  (void) pvParameters;
  pinMode(LED_BUILTIN, OUTPUT);

  for (;;)
  {
    digitalWrite(LED_BUILTIN, HIGH);
    vTaskDelay( 1000 / portTICK_PERIOD_MS );
    digitalWrite(LED_BUILTIN, LOW);
    vTaskDelay( 1000 / portTICK_PERIOD_MS );
  }
}

void TaskAnalogRead(void *pvParameters)
{
  (void) pvParameters;
  
  for (;;)
  {
    digitalWrite(Ci_Pin11, !digitalRead(Ci_Pin11));
    sensorValue = analogRead(A0);

    Serial.print(F("A0: "));
    Serial.println(sensorValue);
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

void vNewTaskAnalogRead(void *pvParameters)
{
  (void) pvParameters;
  unsigned long ul_microsADC = 0;
  
  // Buffer estático: economiza 128 bytes da pilha (stack) da tarefa
  static int16_t i16_ADCReadBuff[DEF_NUMAQUISITION];

  for (;;)
  {
    digitalWrite(Ci_Pin12, HIGH);
    ul_microsADC = micros();

    // 1. Amostragem contínua sem preempção (64 amostras a cada 260 µs)
    vTaskSuspendAll();
    for (int8_t i = 0; i < DEF_NUMAQUISITION;) {
      if (micros() - ul_microsADC >= 260) {
        ul_microsADC = micros();
        digitalWrite(Ci_Pin12, !digitalRead(Ci_Pin12));
        i16_ADCReadBuff[i++] = analogRead(A1);
      }
    }
    // 2. Retoma o escalonador IMEDIATAMENTE após a aquisição dos dados
    xTaskResumeAll();

    digitalWrite(Ci_Pin12, LOW);

    // 3. Atualiza a leitura global de A1 para o display LCD
    sensorValueA1 = i16_ADCReadBuff[DEF_NUMAQUISITION - 1];

    // 4. Transmissão serial eficiente (sem duplicar incremento e sem usar classe String)
    for (int8_t i = 0; i < DEF_NUMAQUISITION; i++) {
      Serial.print(F("Aquisicao "));
      Serial.print(i);
      Serial.print(F(" : "));
      Serial.println(i16_ADCReadBuff[i]);
    }

    // 5. Cede CPU para evitar inanição das tarefas de menor prioridade
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

void TaskDisplay(void *pvParameters)
{
  (void) pvParameters;
  const int ipinDB7 = 23, ipinDB6 = 25, ipinDB5 = 27, ipinDB4 = 29;
  const int ipinE = 39, ipinRW = 41, ipinRS = 43, ipinV0 = 45;
  const int ipinVDD = 47, ipinVSS = 49, ipin15 = 51, ipin16 = 53;

  pinMode(ipinVDD, OUTPUT); digitalWrite(ipinVDD, HIGH);
  pinMode(ipinVSS, OUTPUT); digitalWrite(ipinVSS, LOW);
  pinMode(ipin15, OUTPUT); digitalWrite(ipin15, LOW);
  pinMode(ipin16, OUTPUT); digitalWrite(ipin16, HIGH);

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

  for (;;)
  {
    lcd.setCursor(0, 0);
    lcd.print("A0: ");
    lcd.print(sensorValue);
    lcd.print("    ");

    lcd.setCursor(0, 1);
    lcd.print("A1: ");
    lcd.print(sensorValueA1);
    lcd.print("    ");

    vTaskDelay(pdMS_TO_TICKS(500));
  }
}