#include <stdlib.h> // malloc e free

int16_t *it16_adcdata;
int16_t i16_adcdataleng;

#define DEF_NUM_ADC_CHANNELS 10

typedef union _stAQData{
  int16_t *i16_vectADC;
  uint16_t ui16_crc;
} stAQData;

typedef struct _stADC{
  int16_t i16_nchan;
  int32_t i32_tAQ;
  stAQData *s_adcData;
} stADC;

stADC stADC0 = {.i16_nchan = 10,.i32_tAQ = 5000, .s_adcData = NULL};

void setup() {
  Serial.begin(9600);
  stADC0.s_adcData = malloc(20*sizeof(stAQData));
  for(uint16_t u16_k; u16_k <20;u16_k++){
    stADC0.s_adcData[u16_k].i16_vectADC = malloc(2*stADC0.i16_nchan);
  }
  // put your setup code here, to run once:
}

uint32_t ui32_Time_VCC = 0;
uint32_t ui32_Time_ADCQ = 0,ui32_Time_ADCQ_max = 100;

void loop() {
  if(millis() - ui32_Time_VCC > 1000){
    ui32_Time_VCC = millis();
    
  }

  if(millis() - ui32_Time_ADCQ > ui32_Time_ADCQ_max){
    ui32_Time_ADCQ = millis();

  }

  for(uint16_t u16_k; u16_k <20;u16_k++){
    Serial.print(String(u16_k)+":");
    for(uint16_t u16_k_2; u16_k_2 <20;u16_k_2++){
      Serial.print(stADC0.s_adcData[u16_k].i16_vectADC[u16_k_2]+";");
    }
    Serial.print("\n");
  }
}


int16_t i16_readBandGap()
{
  ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(2);
  ADCSRA |= _BV(ADSC);
  while(bit_is_set(ADCSRA, ADSC));
  return (ADCH << 8) | ADCL;
}
