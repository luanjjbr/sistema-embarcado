# ⏱️ Diretrizes de FreeRTOS para Sistemas Embarcados

## 1. Dimensionamento de Pilhas (Stacks)
- No FreeRTOS para arquitetura AVR (8 bits), cada unidade de stack equivale a **1 word (2 bytes)**.
- Pilha padrão recomendada para tarefas simples: **128 words** (256 bytes):
  - `STACK_BLINK`: 128 words;
  - `STACK_CABO`: 128 words.

## 2. Temporização Não-Bloqueante
- O uso de `delay()` bloqueante é expressamente vedado com o kernel do RTOS em execução.
- Utilizar exclusivamente:
  - `vTaskDelay(pdMS_TO_TICKS(ms))` para atrasos relativos;
  - `vTaskDelayUntil(&xLastWakeTime, xFrequency)` para periodicidade determinística rígida.

## 3. Seções Críticas e Compartilhamento de Recursos
- Proteger variáveis voláteis compartilhadas com semáforos mutex (`xSemaphoreCreateMutex()`) ou desabilitação momentânea de interrupções (`taskENTER_CRITICAL()` / `taskEXIT_CRITICAL()`).
