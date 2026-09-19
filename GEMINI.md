# Diretrizes do Projeto - Sistemas Embarcados

## Idioma de Comunicação
- O assistente deve se comunicar e responder sempre em **Português do Brasil (pt-BR)**.
- Explicações, comentários de código e mensagens de commit/documentação devem ser redigidos em português, mantendo os termos técnicos universais (ex: *baud rate*, *buffer circular*, *checksum*, *CRC-16*, *dangling pointer*, etc.).

## Boas Práticas do Repositório
- Manter código em C / C++ (Arduino/AVR/FreeRTOS) limpo, modular e eficiente.
- Priorizar temporização não-bloqueante (`millis`) e manipulação direta de registradores quando apropriado.
- Documentar funções e rotinas de hardware claramente.
