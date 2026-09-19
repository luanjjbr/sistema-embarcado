#include <Arduino_FreeRTOS.h>
#include <LiquidCrystal.h>

int sensorValue = 0;
// define two tasks for Blink & AnalogRead
void TaskBlink( void *pvParameters );
void TaskAnalogRead( void *pvParameters );
void TaskDisplay( void *pvParameters );

// the setup function runs once when you press reset or power the board
void setup() {
  
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
  
  while (!Serial) {
    ;
  }
  xTaskCreate(TaskBlink,  "Blink",  128,  NULL,  2 ,  NULL );

  xTaskCreate(TaskAnalogRead,  "AnalogRead",  128,  NULL,  1,  NULL );

  xTaskCreate(TaskDisplay,  "Display",  128,  NULL,  1,  NULL );
}

void loop()
{
}

/*--------------------------------------------------*/
/*---------------------- Tasks ---------------------*/
/*--------------------------------------------------*/

void TaskBlink(void *pvParameters)  // This is a task.
{
  (void) pvParameters;
  // initialize digital LED_BUILTIN on pin 13 as an output.
  pinMode(LED_BUILTIN, OUTPUT);

  for (;;) // A Task shall never return or exit.
  {
    digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
    vTaskDelay( 1000 / portTICK_PERIOD_MS ); // wait for one second
    digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
    vTaskDelay( 1000 / portTICK_PERIOD_MS ); // wait for one second
  }
}

void TaskAnalogRead(void *pvParameters)  // This is a task.
{
  (void) pvParameters;
  
  for (;;)
  {
    // read the input on analog pin 0:
    sensorValue = analogRead(A0);
    // print out the value you read:
    Serial.println(sensorValue);
    vTaskDelay(1);  // one tick delay (15ms) in between reads for stability
  }
}

void TaskDisplay(void *pvParameters)  // This is a task.
{
  (void) pvParameters;
  const int ipinDB7 = 23,ipinDB6 = 25,ipinDB5 = 27,ipinDB4 = 29;
  const int ipinE = 39,ipinRW = 41,ipinRS = 43,ipinV0 = 45;
  const int ipinVDD = 47,ipinVSS = 49,ipin15 = 51,ipin16 = 53;

  pinMode(ipinVDD, OUTPUT);digitalWrite(ipinVDD, HIGH);
  pinMode(ipinVSS, OUTPUT);digitalWrite(ipinVSS, LOW);
  pinMode(ipin15, OUTPUT);digitalWrite(ipin15, LOW);
  pinMode(ipin16, OUTPUT);digitalWrite(ipin16, HIGH);

  for(uint16_t i = ipinDB7;i<= ipinDB4;i +=2)
  {
    pinMode(i, OUTPUT);
  }
  for(uint16_t i = ipinE;i<= ipinV0;i +=2)
  {
    pinMode(i, OUTPUT);
  }
  LiquidCrystal lcd(ipinRS, ipinRW, ipinE, ipinDB4, ipinDB5, ipinDB6, ipinDB7);

  lcd.begin(16,2);
  lcd.print("hello, world1!");
  lcd.setCursor(0,1);
  lcd.print("hello, world2!");
  analogWrite(ipinV0, 64);
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  lcd.clear();
  for (;;)
  {
    lcd.setCursor(0,0);
    lcd.print("A0:");
    lcd.setCursor(0,1);
    lcd.print(sensorValue);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}