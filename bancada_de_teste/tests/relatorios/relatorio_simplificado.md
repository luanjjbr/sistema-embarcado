# Relatório de Teste da Bancada

**Data e Hora:** 19/09/2026 às 21:43:41 | **Porta Serial:** `COM5` (115200 bps)  
**Resultado Geral:** **9/9 passos aprovados (100%) em 2 cenários**

---

### 🧪 Cenario 1: Emulacao de Cabos de Recarga (Pino PP)

| # | Nome do Passo | Horário | Comando Enviado | Resposta Recebida | Tempo | Status |
| :---: | :--- | :---: | :--- | :--- | :---: | :---: |
| 1 | **Ativar Cabo Desconectado (S1 - 4700R)** | 21:43:36 | `cabo 1` | `OK: CABO 1 (S1 ATIVA: 4700 Ohms (Desconectado nominal))` | 625 ms | ✅ PASSOU |
| 2 | **Ativar Cabo de 13A (S2 - 1500R)** | 21:43:36 | `cabo 2` | `OK: CABO 2 (S2 ATIVA: 1500 Ohms (Cabo de 13 A))` | 627 ms | ✅ PASSOU |
| 3 | **Ativar Cabo de 20A (S3 - 680R)** | 21:43:37 | `cabo 3` | `OK: CABO 3 (S3 ATIVA:  680 Ohms (Cabo de 20 A))` | 624 ms | ✅ PASSOU |
| 4 | **Ativar Cabo de 32A (S4 - 220R)** | 21:43:37 | `cabo 4` | `OK: CABO 4 (S4 ATIVA:  220 Ohms (Cabo de 32 A))` | 621 ms | ✅ PASSOU |
| 5 | **Ativar Cabo de 63A (S5 - 100R)** | 21:43:38 | `cabo 5` | `OK: CABO 5 (S5 ATIVA:  100 Ohms (Cabo de 63 A / 70 A))` | 625 ms | ✅ PASSOU |
| 6 | **Simular Falha no Cabo (S6 - 47R)** | 21:43:39 | `cabo 6` | `OK: CABO 6 (S6 ATIVA:   47 Ohms (Falha / Curto-circuito))` | 631 ms | ✅ PASSOU |
| 7 | **Desligar Cabo (Circuito Aberto)** | 21:43:39 | `cabo 0` | `OK: CABO 0 (PADRAO (Aberto Total / Nenhuma chave ligada))` | 627 ms | ✅ PASSOU |

### 🧪 Cenario 2: Validacao de Seguranca e Tratamento de Erros

| # | Nome do Passo | Horário | Comando Enviado | Resposta Recebida | Tempo | Status |
| :---: | :--- | :---: | :--- | :--- | :---: | :---: |
| 1 | **Rejeitar Cabo Inexistente (99)** | 21:43:40 | `cabo 99` | `ERRO: Cabo inexistente '99'. Opcoes validas: 0 a 6.` | 632 ms | ✅ PASSOU |
| 2 | **Rejeitar Comando Desconhecido** | 21:43:41 | `comando_invalido` | `ERRO: Comando desconhecido 'comando_invalido'. Digite 'help' para comandos.` | 633 ms | ✅ PASSOU |
