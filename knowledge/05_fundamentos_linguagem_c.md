# 🧩 Conhecimento Consolidado: Fundamentos de C para Embarcados

Baseado no livro: *Head First C* (David Griffiths & Dawn Griffiths, O'Reilly) com foco em sistemas embarcados.

---

## 1. Organização da Memória no Modelo C
1. **Text/Code Segment:** Instruções binárias do programa compilado (alocado na Flash/ROM).
2. **Data Segment (Inicializado):** Variáveis globais e estáticas com valores definidos em compilação.
3. **BSS Segment (Não inicializado):** Variáveis globais e estáticas sem valor inicial explícito (zeradas na inicialização pelo startup code).
4. **Heap Segment:** Memória alocada dinamicamente em tempo de execução (`malloc`, `calloc`). Cresce em direção a endereços altos.
5. **Stack Segment:** Variáveis locais automáticas, ponteiros de retorno e contexto de funções. Cresce em direção a endereços baixos.

---

## 2. Ponteiros, Aritmética e Estruturas
- **Ponteiro e Endereço:** Um ponteiro armazena o endereço físico de uma região de memória (`int *ptr = &val;`).
- **Aritmética de Ponteiros:** `ptr + 1` avança exatamente `sizeof(*ptr)` bytes na memória física.
- **Passagem por Referência:** Permite que funções modifiquem o estado de variáveis do chamador ou passem buffers e structs sem cópia onerosa de bytes na pilha (*stack*).
- **Structs e Alinhamento:** Estruturas agrupam variáveis heterogêneas. Em arquiteturas embarcadas de 8, 16 ou 32 bits, é imperativo atentar ao alinhamento de memória e *padding*.
- **Unions:** Permitem compartilhar a mesma região física de memória entre diferentes tipos, essencial para desempacotar bytes de protocolos e registradores de hardware:
```c
typedef union {
    uint16_t valor;
    struct {
        uint8_t low_byte;
        uint8_t high_byte;
    } bytes;
} registrador_t;
```
- **Ponteiros de Função:** Utilizados para implementar *callbacks*, tabelas de despacho de comandos e máquinas de estados hierárquicas limpas, substituindo cadeias extensas de `switch-case`.

---

## 3. Boas Práticas e Segurança em Firmware
- **Uso do Qualificador `volatile`:** Obrigatório para registradores mapeados em memória de hardware e variáveis globais modificadas dentro de ISRs ou tarefas concorrentes. Informa ao compilador para nunca aplicar otimizações em registradores da CPU sobre aquela variável.
- **Qualificador `const`:** Garante imutabilidade e permite que tabelas de calibração/sensores fiquem na memória Flash (PROGMEM no AVR) sem consumir os escassos bytes de SRAM.
- **Prevenção a Buffer Overflow:** Proibido uso de funções inseguras (`gets`, `strcpy`, `sprintf`). Utilizar estritamente variantes com limite explícito de buffer (`snprintf`, `strncpy`).
- **Verificação de Ponteiros Nulos:** Sempre verificar ponteiros recebidos por parâmetro antes de desreferenciá-los (`if (ptr == NULL) return ERRO;`).
- **Prevenção contra Dangling Pointers:** Após liberar memória (`free()`), sempre atribuir o ponteiro para `NULL` para evitar acessos posteriores acidentais a endereços inválidos.
