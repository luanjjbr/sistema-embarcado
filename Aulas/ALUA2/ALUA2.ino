void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  int i16_ADC_1v1BG = i16_readBandGap();
  Serial.println("ADC_1v1BG: "+String(i16_ADC_1v1BG));
  int32_t i32_calcTemp = 1023L * 1100L;
  i32_calcTemp = i32_calcTemp / i16_ADC_1v1BG;
  Serial.println("ADC_VDD: "+String(i32_calcTemp));

  delay(1000);
}
int16_t i16_readBandGap()
{
  ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(2);
  ADCSRA |= _BV(ADSC);
  while(bit_is_set(ADCSRA, ADSC));
  return (ADCH << 8) | ADCL;

}