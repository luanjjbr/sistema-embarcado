# ⚙️ Conhecimento Consolidado: FreeRTOS Kernel e Guia de Referência

Baseado nos manuais oficiais: *Mastering the FreeRTOS Real Time Kernel* e *FreeRTOS Reference Manual V10.0.0*.

---

## 1. Ciclo de Vida e Estados de Tarefas
Uma tarefa (*Task*) pode estar em um dos quatro estados fundamentais:
- **Running (Em Execução):** A tarefa que detém a CPU no momento.
- **Ready (Pronta):** Pronta para executar, aguardando escalonamento por prioridade.
- **Blocked (Bloqueada):** Aguardando um evento temporal (`vTaskDelay`, `vTaskDelayUntil`) ou evento de sincronização (fila, semáforo, notificação). Não consome tempo de CPU.
- **Suspended (Suspensa):** Explicitamente suspensa via `vTaskSuspend()`, retomada apenas com `vTaskResume()`.

### 1.1. Criação e Dimensionamento de Pilha
```c
BaseType_t xTaskCreate(
    TaskFunction_t pvTaskCode,
    const char * const pcName,
    configSTACK_DEPTH_TYPE usStackDepth, // Em words (2 bytes no AVR, 4 bytes no ARM/ESP32)
    void *pvParameters,
    UBaseType_t uxPriority,              // 0 (mais baixa) a configMAX_PRIORITIES - 1
    TaskHandle_t *pxCreatedTask
);
```

### 1.2. Temporização Precisa e Não-Bloqueante
- `vTaskDelay(pdMS_TO_TICKS(ms))`: Atraso relativo a partir do momento da chamada.
- `vTaskDelayUntil(&xLastWakeTime, xFrequency)`: Temporização periódica determinística e imune a jitter de execução.

---

## 2. Comunicação Intertarefas (IPC) e Sincronização

### 2.1. Filas (Queues)
- Transferência de dados segura por cópia de valor em buffer de anel interno.
- `xQueueCreate(uxQueueLength, uxItemSize)`: Criação da fila.
- `xQueueSend(xQueue, &item, xTicksToWait)`: Envio para a cauda da fila com timeout configurável.
- `xQueueReceive(xQueue, &buffer, xTicksToWait)`: Recepção e remoção do item da fila.
- `xQueuePeek(xQueue, &buffer, xTicksToWait)`: Leitura sem remoção do item da fila.
- `xQueueOverwrite(xQueue, &item)`: Gravação forçada em filas de tamanho 1 (ideal para diagnóstico contínuo).

### 2.2. Semáforos e Mutexes
- **Semáforo Binário (`xSemaphoreCreateBinary`):** Utilizado para sincronização de eventos entre interrupções e tarefas.
- **Semáforo Contador (`xSemaphoreCreateCounting`):** Controle de pool de recursos finitos ou contagem de eventos.
- **Mutex (`xSemaphoreCreateMutex`):** Exclusão mútua com *Priority Inheritance* (Herança de Prioridade) para mitigar inversão de prioridade ilimitada.
- **Regra Fundamental:** Nunca invocar chamadas de mutex a partir de rotinas de interrupção (ISRs). Mutexes são exclusivos do contexto de tarefas.

### 2.3. Notificações Diretas de Tarefa (Task Notifications)
- Mecanismo ultrarrápido de sinalização associado diretamente ao TCB (Task Control Block) de cada tarefa.
- Economiza até 45% de RAM em comparação com semáforos convencionais.
- Primitivas: `xTaskNotifyGive()`, `ulTaskNotifyTake()`, `xTaskNotify()`, `xTaskNotifyWait()`.

---

## 3. Tratamento de Interrupções e Determinismo (ISR)
- Todas as funções do kernel invocadas em rotinas de interrupção devem conter o sufixo `FromISR`.
- Sinalização de troca de contexto para retorno determinístico imediato:
```c
BaseType_t xHigherPriorityTaskWoken = pdFALSE;
xQueueSendFromISR(xQueue, &dado, &xHigherPriorityTaskWoken);
portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
```

---

## 4. Gerenciamento de Memória (Heap Allocators)
- **`heap_1`:** Alocação simples em array estático. Não permite desalocação (`vPortFree` não implementada). Indicado para sistemas com alocação estática no startup.
- **`heap_2`:** Permite alocação e liberação usando algoritmo de melhor ajuste (*best fit*), mas sem fusão de blocos adjacentes (suscetível a fragmentação).
- **`heap_3`:** Wrapper *thread-safe* para o `malloc()` e `free()` da biblioteca padrão de C.
- **`heap_4`:** Algoritmo de primeiro ajuste (*first fit*) com fusão (*coalescing*) de blocos de memória livres adjacentes. Recomendado para uso geral.
- **`heap_5`:** Funciona de forma similar ao `heap_4`, porém suporta heaps particionadas em múltiplos blocos físicos de memória não contíguos.
- **Detecção de Stack Overflow:**
  - `configCHECK_FOR_STACK_OVERFLOW = 1`: Verifica se o stack pointer da tarefa ultrapassou os limites do bloco alocado.
  - `configCHECK_FOR_STACK_OVERFLOW = 2`: Preenche o final da pilha com padrão conhecido (0xA5) e valida se os últimos bytes foram corrompidos.
  - Implementação obrigatória da função de gancho: `void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)`.
