#include <EEPROM.h>

#define EEPROM_ADDR_CFG 0

typedef struct {
  uint16_t u16_num_canais;
  uint16_t u16_tempo_aquisicao;
  uint16_t u16_tempo_uart;
  uint16_t u16_checksum;
} st_cfg_t;

typedef union {
  int16_t i16_canais[9];
  struct{
    uint32_t i16_timestamp;
    int16_t i16_a0;
    int16_t i16_a1;
    int16_t i16_a2;
    int16_t i16_a3;
    int16_t i16_a4;
    int16_t i16_a5;
    int16_t i16_a6;
    int16_t i16_a7;
  }st_adc_t;
} st_channel_t;

uint8_t u8_indice_log = 0;
char buf_uart[32];
uint8_t u8_idx_uart = 0;

void salvarConfig(st_cfg_t &cfg) {
  cfg.u16_checksum = calcularChecksum(cfg);
  EEPROM.put(EEPROM_ADDR_CFG, cfg);
}//end salvarConfig

bool carregarConfig(st_cfg_t &cfg) {
  EEPROM.get(EEPROM_ADDR_CFG, cfg);
  return (calcularChecksum(cfg) == cfg.u16_checksum);
}//end carregarConfig

void print_data(st_channel_t *st_dado, uint8_t u8_num_logs, uint8_t u8_canais)
{
    for (uint8_t u8_i = 0; u8_i < u8_num_logs; u8_i++)
    {
        for (uint8_t u8_j = 0; u8_j < u8_canais + 1; u8_j++)
        {
            Serial.print(st_dado[u8_i].i16_canais[u8_j]);
            if (u8_j < u8_canais)
            {
                Serial.print(",");
            }
        }
        Serial.print("\n");
    }
    Serial.print("---------------------------\n");
}//end print_data

void print_data_2(st_channel_t *st_dado, uint8_t u8_num_logs, uint8_t u8_canais, uint8_t u8_indice_inicial)
{
    for (uint8_t u8_k = 0; u8_k < u8_num_logs; u8_k++)
    {
        uint8_t u8_i = (u8_indice_inicial + u8_k) % u8_num_logs;   // <-- comeca do dado mais antigo, roda em circulo

        for (uint8_t u8_j = 0; u8_j < u8_canais + 1; u8_j++)
        {
            Serial.print(st_dado[u8_i].i16_canais[u8_j]);
            if (u8_j < u8_canais)
            {
                Serial.print(",");
            }
        }
        Serial.print("\n");
    }
    Serial.print("---------------------------\n");
}//end print_data

uint16_t calcularChecksum(const st_cfg_t &cfg) {
  const uint8_t *p = (const uint8_t*)&cfg;
  uint16_t chk = 0;
  for (size_t i = 0; i < offsetof(st_cfg_t, u16_checksum); i++) {
    chk += p[i];   // soma todos os bytes, exceto o campo checksum
  }
  return chk;
} // end calcularChecksum

st_cfg_t cfg;

st_channel_t st_log_canais[5];

uint32_t u32_tempo_aquisicao = 0;
uint32_t u32_tempo_uart = 0;

void setup() {
  Serial.begin(9600);
  if (!carregarConfig(cfg)) {
    delay(1000);
    Serial.println("EEPROM corrompida ou vazia! Usando valores padrao.");
    cfg.u16_num_canais = 8;
    cfg.u16_tempo_aquisicao = 1;
    cfg.u16_tempo_uart = 5;
    salvarConfig(cfg);   // grava os defaults de volta, ja com checksum valido
  }
  u32_tempo_aquisicao = millis();
  u32_tempo_uart = millis();

  Serial.print("Ncanal: ");
  Serial.println(cfg.u16_num_canais);

  Serial.print("Tempo de aquisicao: ");
  Serial.println(cfg.u16_tempo_aquisicao);

  Serial.print("Tempo de UART: ");
  Serial.println(cfg.u16_tempo_uart);
}// end setup

void verificarComandoUART() {
  while (Serial.available()) {
    char c = Serial.read();

    if (c == '\n' || c == '\r') {
      if (u8_idx_uart == 0) continue;
      buf_uart[u8_idx_uart] = '\0';

      uint16_t v1, v2, v3;
      int pos = 0;
      int n = sscanf(buf_uart, "%hu,%hu,%hu%n", &v1, &v2, &v3, &pos);

      u8_idx_uart = 0;

      // formato precisa ser EXATAMENTE "valor,valor,valor", sem sobra de texto
      bool formato_ok = (n == 3) && ((size_t)pos == strlen(buf_uart));

      // limites de cada campo
      bool limites_ok = formato_ok
                      && (v1 >= 1 && v1 <= 8)    // canais: 1 a 8
                      && (v2 >= 1 && v2 <= 60)   // tempo_aquisicao: 1 a 60
                      && (v3 >= 1 && v3 <= 60);  // tempo_uart: 1 a 60

      if (limites_ok) {
        cfg.u16_num_canais      = v1;
        cfg.u16_tempo_aquisicao = v2;
        cfg.u16_tempo_uart      = v3;
        salvarConfig(cfg);

        memset(st_log_canais, 0, sizeof(st_log_canais));
        u8_indice_log = 0;

        Serial.println("Config atualizada e salva!");
      }
      else if (!formato_ok) {
        Serial.println("Formato invalido. Use: canais,aquisicao,uart");
      }
      else {
        Serial.println("Fora dos limites: canais 1-8, tempos 1-60");
      }
    }
    else {
      if (u8_idx_uart < sizeof(buf_uart) - 1) {
        buf_uart[u8_idx_uart++] = c;
      }
    }
  }
}//end verificarComandoUART

void ler_canais(st_channel_t &canal, uint8_t u8_num_canais) {
  //canal.i16_canais[0] ++;   // timestamp sempre gravado
  canal.i16_canais[0] = millis()/1000;   // timestamp sempre gravado

  for (uint8_t u8_i = 0; u8_i < u8_num_canais; u8_i++) {
    canal.i16_canais[u8_i + 1] = analogRead(u8_i);   // A0, A1... ate o numero configurado
  }
}//end ler_canais

void loop() {
  uint32_t u32_tempo_atual = millis();
  
  verificarComandoUART();

  // timer de aquisição
  if ((u32_tempo_atual - u32_tempo_aquisicao) >= (cfg.u16_tempo_aquisicao * 1000UL)) {
    u32_tempo_aquisicao = u32_tempo_atual;

    ler_canais(st_log_canais[u8_indice_log],cfg.u16_num_canais);
    u8_indice_log++;
    if (u8_indice_log >= 5) u8_indice_log = 0;   // buffer circular
  }// end if aquisição

  // timer de UART
  if ((u32_tempo_atual - u32_tempo_uart) >= (cfg.u16_tempo_uart * 1000UL)) {
    u32_tempo_uart = u32_tempo_atual; 
    //print_data(st_log_canais, sizeof(st_log_canais)/sizeof(st_log_canais[0]), cfg.u16_num_canais);
    print_data_2(st_log_canais, sizeof(st_log_canais)/sizeof(st_log_canais[0]), cfg.u16_num_canais, u8_indice_log);
  }// end if UART

  // static uint32_t u32_tempo_uart_2 = 0;
  // timer de UART
  // if ((u32_tempo_atual - u32_tempo_uart_2) >= (5 * 1000UL)) {
  //   u32_tempo_uart_2 = u32_tempo_atual; 
  //   print_data(st_log_canais, 5, 8);
  // }// end if UART

}// end loop