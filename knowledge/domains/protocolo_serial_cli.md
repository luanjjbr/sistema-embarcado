# 💬 Domínio: Protocolo de Comunicação Serial (CLI)

## 1. Parâmetros de Linha
- Baud Rate: **`9600 bps`** (Bancada) / **`115200 bps`** (Bancada de Teste ESP32).
- Formato: 8 bits de dados, sem paridade, 1 stop bit (8N1).
- Terminador: `\n` ou `\r\n`.

## 2. Tabela de Comandos
| Comando Recebido | Ação Executada | Resposta Serial |
| :--- | :--- | :--- |
| `cabo 0` a `cabo 6` | Ativa o canal correspondente do barramento PP | `OK\r\n` |
| `cabo <invalido>` | Canal fora da faixa permitida | `ERRO: Cabo\r\n` |
| `help` ou `?` | Imprime tela de ajuda com lista de comandos | Menu formatado |
| `<desconhecido>` | Comando não reconhecido | `ERRO\r\n` |
