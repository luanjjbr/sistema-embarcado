// ============================================================
// Estrutura dos dados
// ============================================================

typedef union _st_data
{
    int16_t i16_dados[9];
    struct _st_adc {
        int32_t i32_Time;
        int32_t i32_adc0;
        int32_t i32_adc1;
        int32_t i32_adc2;
        int32_t i32_adc3;
        int32_t i32_adc4;
        int32_t i32_adc5;
        int32_t i32_adc6;
        int32_t i32_adc7;
    }st_adc;

} st_data;

// ============================================================
// Estrutura do Data Logger
// ============================================================

typedef struct _st_data_logger_t
{
    uint8_t u8_canais;
    float f32_tempo_aquisicao;
    float f32_tempo_transferencia;
    st_data st_dado[8];
} st_data_logger_t;

// ============================================================
// Data Logger
// ============================================================
static st_data_logger_t Log = {2, 1.0, 5, {0}};

// ============================================================
// Temporizadores
// ============================================================
uint32_t u32_tempo_aquisicao = 0;
uint32_t u32_tempo_uart = 0;

// ============================================================
// Função para mover as aquisições
// ============================================================
void move(st_data st_dado[8])
{
    for (uint8_t u8_i = 7; u8_i > 0; u8_i--)
    {
        st_dado[u8_i] = st_dado[u8_i - 1];
    }
}

// ============================================================
// Função para imprimir os dados
// ============================================================
void print_data(st_data st_dado[8], uint8_t u8_canais)
{
    for (uint8_t u8_i = 0; u8_i < 8; u8_i++)
    {
        for (uint8_t u8_j = 0; u8_j < u8_canais+1; u8_j++)
        {
            Serial.print(st_dado[u8_i].i16_dados[u8_j]);
            if (u8_j < u8_canais)
            {
                Serial.print(",");
            }
        }
        Serial.print("\n");
    }
    Serial.print("---------------------------\n");
}


// ============================================================
// Limites
// ============================================================

static const uint8_t u8_min_canais = 1;
static const uint8_t u8_max_canais = 8;

static const float f32_min_tempo_aquisicao = 0.5;
static const float f32_max_tempo_aquisicao = 10.0;

static const float f32_min_tempo_uart = 0.5;
static const float f32_max_tempo_uart = 10.0;

// ============================================================
// Função para receber configuração pela UART
// ============================================================

void uart_config()
{
    while (Serial.available() > 0)
    {
        char c_caracter = Serial.read();
        if (c_caracter == '\n' || c_caracter == '\r'){
            Serial.println(c_caracter);
        }
    }
}
// ============================================================
// Setup
// ============================================================

void setup()
{
    Serial.begin(9600);
    u32_tempo_aquisicao = millis();
    u32_tempo_uart = millis();
}

// ============================================================
// Loop
// ============================================================

void loop()
{
    uint32_t u32_tempo_atual = millis();

    // ========================================================
    // Aquisição dos dados
    // ========================================================

    if ((u32_tempo_atual - u32_tempo_aquisicao) >= (Log.f32_tempo_aquisicao * 1000))
    {
        move(Log.st_dado);
        //Log.st_dado[0].i16_dados[0] = u32_tempo_atual;
        Log.st_dado[0].i16_dados[0] ++;
        for (uint8_t u8_i = 0; u8_i < Log.u8_canais; u8_i++)
        {
            Log.st_dado[0].i16_dados[u8_i+1] = analogRead(u8_i);
        }

        u32_tempo_aquisicao = u32_tempo_atual;
    }

    // ========================================================
    // Transferência pela UART
    // ========================================================
    if ((u32_tempo_atual - u32_tempo_uart) >= (Log.f32_tempo_transferencia * 1000))
    {
        print_data(Log.st_dado, Log.u8_canais);

        u32_tempo_uart = u32_tempo_atual;
    }


    uart_config();
}