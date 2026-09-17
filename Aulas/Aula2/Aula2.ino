#include <Arduino.h>

// Função para ler a tensão interna de referência (1.1V Bandgap)
int16_t i16_readBandGap()
{
    // Seleciona Vcc como referência (REFS0) e canal MUX para a Bandgap de 1.1V (MUX3, MUX2, MUX1)
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
    
    // Pequeno tempo para estabilização da tensão de referência interna
    delay(2);
    
    // Inicia a conversão A/D (ADSC = ADC Start Conversion)
    ADCSRA |= _BV(ADSC);
    
    // Aguarda a conclusão da conversão
    while (bit_is_set(ADCSRA, ADSC));
    
    // ADCW lê automaticamente ADCL primeiro e depois ADCH na ordem correta exigida pelo AVR
    return ADCW; 
}

void setup() {
    Serial.begin(9600);
}

void loop() {
    // Leitura do valor bruto do ADC para a referência interna de 1.1V
    int16_t i16_ADC_1v1BG = i16_readBandGap();
    
    // Impressão da leitura bruta
    Serial.print("ADC_1v1BG: ");
    Serial.println(i16_ADC_1v1BG);

    // Cálculo da tensão de alimentação VDD em mV:
    // VDD = (1023 * 1100 mV) / Leitura_ADC
    if (i16_ADC_1v1BG > 0) 
    {
        int32_t i32_calcVDD = 1023L * 1100L;
        i32_calcVDD = i32_calcVDD / i16_ADC_1v1BG;
        
        Serial.print("ADC_VDD (mV): ");
        Serial.println(i32_calcVDD);
    }
    else 
    {
        Serial.println("Erro na leitura do ADC!");
    }

    delay(1000);
}