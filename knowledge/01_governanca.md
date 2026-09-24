# 🏛️ Governança e Regras Operacionais dos Agentes

## 1. Metodologia Human-in-the-Loop (Turno Separado)
- Todo agente operando neste repositório deve seguir o ciclo obrigatório:
  1. **Explicação Técnica Prévia:** Explicar detalhadamente a lógica de software embarcado e de hardware/eletrônica.
  2. **Exibição do Código Proposto:** Apresentar o trecho exato ou arquivo completo que será modificado/criado.
  3. **Solicitação Expressa de Autorização:** Pedir explicitamente permissão ao usuário.
  4. **Parada Obrigatória:** Interromper a execução no turno corrente e aguardar a mensagem do usuário autorizando a escrita. Proibido chamar ferramentas de gravação no mesmo turno.

## 2. Padrão de Versionamento (Conventional Commits)
- `feat:` Nova funcionalidade, aula prática ou driver de periférico;
- `fix:` Correção de bug em firmware, pinagem ou temporização;
- `docs:` Modificação em documentações técnicas e diagramas;
- `refactor:` Refatoração de código sem alteração do comportamento externo;
- `chore:` Manutenção em arquivos de build (`platformio.ini`, git, scripts).

## 3. Idioma e Terminologia
- Estritamente **Português do Brasil (pt-BR)** para explicações, mensagens de commit e documentações.
- Preservação da terminologia técnica universal consolidada (*baud rate*, *buffer circular*, *break-before-make*, *stack overflow*, *task*, *mutex*, etc.).
