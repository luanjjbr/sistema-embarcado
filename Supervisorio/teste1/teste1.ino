// Definições de pinos e intervalos
const int LED_PIN = LED_BUILTIN; // Pino 13 (LED nativo da placa)
const unsigned long INTERVALO_BLINK = 500; // 0.5s
const unsigned long INTERVALO_ENVIO = 3000; // 3.0s

// Variáveis de controle de tempo
unsigned long tempoAnteriorBlink = 0;
unsigned long tempoAnteriorEnvio = 0;
bool estadoLed = LOW;

void setup() {
  pinMode(LED_PIN, OUTPUT);
  
  // Inicializa a comunicação Serial a 9600 bps (igual ao C#)
  Serial.begin(9600); 
}

void loop() {
  unsigned long tempoAtual = millis();

  // 1. BLINK (Pisca o LED a cada 500ms)
  if (tempoAtual - tempoAnteriorBlink >= INTERVALO_BLINK) {
    tempoAnteriorBlink = tempoAtual;
    estadoLed = !estadoLed; // Inverte o estado (HIGH/LOW)
    digitalWrite(LED_PIN, estadoLed);
  }

  // 2. ECHO (Devolve tudo o que recebe pela UART)
  if (Serial.available() > 0) {
    char caractereRecebido = Serial.read();
    Serial.print(caractereRecebido); // Envia de volta o mesmo caractere
  }

  // 3. ENVIO PERIÓDICO (Envia "123" a cada 3 segundos)
  if (tempoAtual - tempoAnteriorEnvio >= INTERVALO_ENVIO) {
    tempoAnteriorEnvio = tempoAtual;
    Serial.println("123"); // Envia "123" com quebra de linha (\r\n)
  }
}