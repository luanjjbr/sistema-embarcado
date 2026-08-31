#include <stdlib.h> // malloc e free

int16_t *it16_adcdata;
int16_t i16_adcdataleng;

#define DEF_NUM_ADC_CHANNELS 10

typedef struct _stAQData{
  int16_t *i16_vectADC;
  uint16_t u16_crc;
  struct _stAQData *nex;
} stAQData;

typedef struct _stADC{
  int16_t i16_nchan;
  int32_t i32_tAQ;
  stAQData *s_adcData;
} stADC;



void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  i16_adcdataleng = DEF_NUM_ADC_CHANNELS;
  it16_adcdata = malloc(2*i16_adcdataleng);
  if(it16_adcdata == -1){
    Serial.println("erro");

  }else{
    Serial.println("ok");
    for(int16_t u16_idx = 0; u16_idx < i16_adcdataleng;u16_idx++){
      it16_adcdata[u16_idx] = (int16_t)(rand()*32767);
    }
    for(int16_t u16_idx = 0; u16_idx < i16_adcdataleng;u16_idx++){
      Serial.println(String(u16_idx)+": "+String(it16_adcdata[u16_idx]));
    }
  }

}

void loop() {


}
