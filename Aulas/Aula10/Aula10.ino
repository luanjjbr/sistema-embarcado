#include <Arduino_FreeRTOS.h>
#include <semphr.h>

#define confgGENERATE_RUN_STATS 1

void vSetupT

int sensorValue = 0;
SemaphoreHandle_t xMutexSerial;
TickType_t xBlockTime = pdMS_TO_TICKS(100); // Tempo de espera para o Mutex

// Declaração das tasks
void TaskBlink(void *pvParameters);
void TaskAnalogRead(void *pvParameters);

void setup() {
  // Inicializa a comunicação serial
  Serial.begin(9600);
  
  while (!Serial) {
    ; // Espera a porta serial conectar (para placas baseadas em 32u4)
  }

  // Criação do Mutex para proteger o recurso compartilhado (Serial)
  xMutexSerial = xSemaphoreCreateMutex();

  if (xMutexSerial != NULL) {
    // Criação da Task de Blink
    xTaskCreate(
      TaskBlink,
      "Blink",
      128,
      NULL,
      1,
      NULL
    );

    // Criação da Task de Leitura Analógica
    xTaskCreate(
      TaskAnalogRead,
      "AnalogRead",
      128,
      NULL,
      1,
      NULL
    );
  }
  // O escalonador do FreeRTOS inicia automaticamente aqui.
}

void loop() {
  // Vazio. Toda a lógica é executada nas Tasks do FreeRTOS.
}

/*--------------------------------------------------*/
/*---------------------- Tasks ---------------------*/
/*--------------------------------------------------*/

void TaskBlink(void *pvParameters) {
  (void) pvParameters;
  pinMode(LED_BUILTIN, OUTPUT);

  for (;;) {
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN)); 

    // Protegendo o acesso à Serial com o Mutex
    if (xSemaphoreTake(xMutexSerial, xBlockTime) == pdTRUE) {
      Serial.println("2-Blink: LED Toggled");
      xSemaphoreGive(xMutexSerial); // Libera o Mutex
    }else{
      Serial.println("2-Blink: LED Toggled");
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS); // Aguarda 1 segundo
  }
}

void TaskAnalogRead(void *pvParameters) {
  (void) pvParameters;

  for (;;) {
    // Lê a entrada analógica no pino A0
    sensorValue = analogRead(A0);

    // Protegendo o acesso à Serial com o Mutex
    if (xSemaphoreTake(xMutexSerial, xBlockTime) == pdTRUE) {
      Serial.println("1-ADC0:" + String(sensorValue));
      xSemaphoreGive(xMutexSerial); // Libera o Mutex
    }

    vTaskDelay(1000 / portTICK_PERIOD_MS); // Aguarda 1 segundo
  }
}