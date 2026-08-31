#include <Arduino.h>
#include <EEPROM.h>
#include <string.h>

#define CMD_BUFFER_SIZE 64
#define EEPROM_DUMP_SIZE 0x40

// Estrutura de dados gravada na EEPROM
typedef struct _StAQ {
    uint32_t ui32_tAQ;
    uint32_t ui32_tUSART;
    uint16_t ui16_ADCHAN;
    uint16_t ui16_CRC16;
} StAQ_t;

typedef union _uEEPROMDATA {
    byte b8_data[sizeof(StAQ_t)];
    StAQ_t StAQ;
} uEEPROMDATA_t;

// Instâncias Globais
uEEPROMDATA_t uEEPROMSAVE = {
    .StAQ = {
        .ui32_tAQ = 0xAAAA5555,
        .ui32_tUSART = 0x00112233,
        .ui16_ADCHAN = 0x5566,
        .ui16_CRC16 = 0x0000
    }
};

char cmdBuffer[CMD_BUFFER_SIZE];
int i_cmdBufferIndex = 0;

// Prototipação de Funções
uint16_t crc16(const uint8_t *data, size_t length);
int16_t i16_gravaEEPDATA(uEEPROMDATA_t *uE);
int16_t i16_GetEEPDATA(uEEPROMDATA_t *uE);
void processCommand(char *cmd);
void v_eeprom_dump(unsigned int ui_length);

// ============================================================
// Algoritmo CRC-16 (Modbus - Polinômio 0xA001)
// ============================================================
uint16_t crc16(const uint8_t *data, size_t length) {
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < length; i++) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}

// ============================================================
// Manipulação de EEPROM com CRC
// ============================================================
int16_t i16_gravaEEPDATA(uEEPROMDATA_t *uE) {
    uint16_t u16_addr = 0;
    size_t data_len = sizeof(StAQ_t);
    
    // Calcula o CRC cobrindo todos os bytes com exceção do próprio campo do CRC (2 bytes finais)
    uE->StAQ.ui16_CRC16 = crc16(uE->b8_data, data_len - sizeof(uint16_t));
    
    for (size_t ui_pos = 0; ui_pos < data_len; ui_pos++) {
        EEPROM.update(u16_addr + ui_pos, uE->b8_data[ui_pos]);
    }
    return 0;
}

int16_t i16_GetEEPDATA(uEEPROMDATA_t *uE) {
    uint16_t u16_addr = 0;
    size_t data_len = sizeof(StAQ_t);

    for (size_t ui_pos = 0; ui_pos < data_len; ui_pos++) {
        uE->b8_data[ui_pos] = EEPROM.read(u16_addr + ui_pos);
    }

    uint16_t ui16_CRC16_temp = crc16(uE->b8_data, data_len - sizeof(uint16_t));

    if (ui16_CRC16_temp != uE->StAQ.ui16_CRC16) {
        // Carrega padrão se falhar
        uE->StAQ.ui32_tAQ = 100;
        uE->StAQ.ui32_tUSART = 5000;
        uE->StAQ.ui16_ADCHAN = 5;
        return -1; // Retorna erro de integridade
    }
    return 0;
}

// ============================================================
// Interface UART (Comandos & Dump)
// ============================================================
void v_eeprom_dump(unsigned int ui_length) {
    const int eepromsize = EEPROM.length();
    const int bytesPerLine = 16;
    
    if (eepromsize < (int)ui_length || eepromsize == 0) ui_length = eepromsize;
    
    Serial.println("\n--- Dump da EEPROM (Hexadecimal) ---");
    for (unsigned int addr = 0; addr < ui_length; addr++) {
        if (addr % bytesPerLine == 0) {
            Serial.print("\n0x");
            if (addr < 0x10) Serial.print("0");
            Serial.print(addr, HEX);
            Serial.print(": ");
        }
        byte value = EEPROM.read(addr);
        if (value < 0x10) Serial.print("0");
        Serial.print(value, HEX);
        Serial.print(" ");
    }
    Serial.println("\n-------------------------------------");
}

void processCommand(char *cmd) {
    if (strcmp(cmd, "dump") == 0) {
        v_eeprom_dump(EEPROM_DUMP_SIZE);
    } else if (strcmp(cmd, "help") == 0) {
        Serial.println("\n--- Comandos Disponiveis ---");
        Serial.println("dump - Exibe o mapa de memoria da EEPROM em Hexadecimal");
        Serial.println("help - Mostra este menu de ajuda");
    } else {
        Serial.print("Comando invalido: ");
        Serial.println(cmd);
    }
}

// ============================================================
// Setup e Loop Principal
// ============================================================
void setup() {
    Serial.begin(9600);
    pinMode(LED_BUILTIN, OUTPUT);

    if (i16_GetEEPDATA(&uEEPROMSAVE) == -1) {
        Serial.println("Falha de CRC na EEPROM! Gravando valores padrao...");
        i16_gravaEEPDATA(&uEEPROMSAVE);
    } else {
        Serial.println("EEPROM lida com sucesso (CRC OK).");
    }
}

void loop() {
    static unsigned long ul_blink_ms = 0;
    static unsigned long ul_parser_ms = 0;

    // Pisca LED indicando operação do sistema
    if (millis() - ul_blink_ms > 1000) {
        ul_blink_ms = millis();
        digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    }

    // Leitura não-bloqueante da Serial
    if (millis() - ul_parser_ms > 50) {
        ul_parser_ms = millis();
        while (Serial.available()) {
            char c = Serial.read();
            if (c == '\n' || c == '\r') {
                if (i_cmdBufferIndex > 0) {
                    cmdBuffer[i_cmdBufferIndex] = '\0';
                    processCommand(cmdBuffer);
                    i_cmdBufferIndex = 0;
                }
            } else {
                if (i_cmdBufferIndex < CMD_BUFFER_SIZE - 1) {
                    cmdBuffer[i_cmdBufferIndex++] = c;
                }
            }
        }
    }
}