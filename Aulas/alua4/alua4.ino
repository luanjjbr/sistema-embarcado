#include <EEPROM.h>
#define DATALEN (4*2+2*2)
#define CMD_BUFFER_SIZE 64
#define EEPROM_DUMP_SAZE 0x40

char cmdBuffer[CMD_BUFFER_SIZE];
int i_cmdBufferIndex = 0;

// gestão da eeprom
typedef union _uEEPROMDATA{
  byte b8_data[DATALEN];
  struct _StAQ{
    uint32_t ui32_tAQ;
    uint32_t ui32_tUSART;
    uint16_t ui16_ADCHAN;
    uint16_t ui16_CRC16;
  }StAQ;
}uEEPROMDATA_t;

uEEPROMDATA_t uEEPROMSAVE{
  .StAQ = {
    .ui32_tAQ=0xAAAA5555,
    .ui32_tUSART=0x00112233,
    .ui16_ADCHAN=0x5566,
    .ui16_CRC16=0x1122
    }
  };
uEEPROMDATA_t  uEEPROMREAD;

typedef struct {
  uint16_t u16_num_canais;
  uint16_t u16_tempo_aquisicao;
  uint16_t u16_tempo_uart;
  uint16_t u16_checksum;
} st_cfg_t;

int16_t i16_gravaEEPDATA(uEEPROMDATA_t *uE){
  uint16_t u16_addr = 0;
  uE->StAQ.ui16_CRC16 = crc16(uE->b8_data,DATALEN- 1*2);
  for(int ui_pos = 0; ui_pos < DATALEN; ui_pos++)
  {
    EEPROM.update(u16_addr, uE->b8_data[ui_pos]);
    u16_addr++;
  }
  return 5;
}

int16_t i16_GetEEPDATA(uEEPROMDATA_t *uE){
  uint16_t u16_addr = 0;
  for(int ui_pos = 0; ui_pos < DATALEN; ui_pos++)
  {
    uE->b8_data[ui_pos] = EEPROM.read(u16_addr);
    u16_addr++;
  }
  //uE->StAQ.ui32_tAQ++; //simular falha
  uint16_t ui16_CRC16_temp = crc16(uE->b8_data,DATALEN- 1*2);
  if(ui16_CRC16_temp != uE->StAQ.ui16_CRC16){
    uE->StAQ.ui32_tAQ = 100;
    uE->StAQ.ui32_tUSART = 5000;
    uE->StAQ.ui16_ADCHAN = 5;
    uE->StAQ.ui16_CRC16 = crc16(uE->b8_data,DATALEN- 1*2);
    return -1;
  }else{
    return 5;
  }
}



void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  //i16_gravaEEPDATA(&uEEPROMSAVE);
  //v_eeprom_dump(EEPROM_DUMP_SAZE);
  pinMode(LED_BUILTIN, OUTPUT);
  if(i16_GetEEPDATA(&uEEPROMSAVE) == -1){
    Serial.println("falha");
    i16_gravaEEPDATA(&uEEPROMSAVE);
  }else{
    Serial.println("ok");
  }

}

void loop() {
  static unsigned long ul_blink_ms = 0;
  static unsigned long ul_parser_ms = 0;
  // put your main code here, to run repeatedly:
  if(millis() - ul_blink_ms>1000){
    ul_blink_ms = millis();
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  }

  if(millis() - ul_parser_ms > 100){
    ul_parser_ms = millis();
    while(Serial.available()){
      char c = Serial.read();
      if(c == '\n' || c =='\r'){
        //Fim de envio de comando
        if(i_cmdBufferIndex > 0){
          cmdBuffer[i_cmdBufferIndex] = '\0';
          processCommand(cmdBuffer);
          i_cmdBufferIndex = 0; //Reset Buffer
        }
      } else {
        // Armazena o caractere no buffer
        if(i_cmdBufferIndex < CMD_BUFFER_SIZE - 1){
          cmdBuffer[i_cmdBufferIndex++] = c;
        }
      }
    }
  }
}

void processCommand(char *cmd){
  if(strcmp(cmd, "dump" )== 0){
    v_eeprom_dump(EEPROM_DUMP_SAZE);
  }
  else if(strcmp(cmd,"help")==0){
    Serial.println("COmandos:");
    Serial.println("dump");
    Serial.println("help");
  }else{
    Serial.println("Erro no comando");
    Serial.println(cmd);
  }
}


void v_eeprom_dump(unsigned int ui_length){
  const int eepromsize = EEPROM.length();
  const int bytesPertLine = 16;
  if(eepromsize < ui_length || eepromsize == 0x0) ui_length = eepromsize;
  Serial.println("Dump da EEProm (Hexadecimal):");
  for(int addr = 0;addr< ui_length; addr++){
    if(addr % bytesPertLine == 0){
      Serial.print("\n 0x");
      if(addr< 0x10)Serial.print("0"); // alinhamento
      Serial.print(addr,HEX);
      Serial.print(": ");
    }
    // le byte da eeprom
    byte value = EEPROM.read(addr);
    // imprime em hexa com dois digitos
    if(value<0x10) Serial.print("0");
    Serial.print(value,HEX);
    Serial.print("; ");
  }
}

uint16_t crc16(const uint8_t *data,size_t length){
  uint16_t crc = 0xFFFF;
  for(size_t i = 0;i < length;i++){
    crc ^= data[i];
    for(uint8_t j = 0; j <8; j++){
      if(crc & 0x0001){
        crc = (crc >> 1) ^0xA001;
      }
      else{
        crc >>= 1;
      }
    }
  }
  return crc;
}