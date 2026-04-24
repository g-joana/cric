# 🚀 ROADMAP - ft_irc (Agentes)

## Visão Geral

Este documento descreve o roadmap para desenvolvimento do servidor IRC `ft_irc` usando **Sessões de Agentes Independentes**.

**Timeline**: 6 Sprints | **Formato**: Agentes paralelos | **Target**: Entrega em 1 maratona (~12.5h)

---

## 📊 Estrutura de Sprints

```
S0 (Blocker)
   ↓
S1 (Crítico: Parser)
   ↓
S2, S3, S4, S5 (Podem rodar em paralelo após S1)
   ↓
S6 (Final: Robustez)
```

---

## 🎯 Sprint Descriptions

### S0 - Investigação: Bug Ctrl+D
**Tempo**: 30min | **Blocker**: ✅ YES

Investigar e documentar o bug onde cliente que desconecta via Ctrl+D bloqueia outros clientes.

**Saída**:
- `docs/sprints_knowledge/S0-BUG-ANALYSIS.md`
- Scripts de reprodução

**Próximo Sprint**: S1 usa análise para corrigir

---

### S1 - Parser IRC + Bug Fix
**Tempo**: 2-3h | **Blocker**: ✗ NO (mas crítico)

Implementar parser robusto que agrega pacotes fragmentados e corrigir bug de S0.

**Saída**:
- Classe CommandParser
- Bug Ctrl+D fixado
- `docs/sprints_knowledge/S1-PARSER-DESIGN.md`

**Próximos Sprints**: S2-S6 dependem do parser

---

### S2 - Autenticação (PASS/NICK/USER)
**Tempo**: 2h | **Blocker**: ✗ NO

Handshake de autenticação: PASS → NICK → USER → RPL_WELCOME (001).

**Saída**:
- Estados de cliente (INIT → AUTH → REGISTERED)
- Handlers PASS/NICK/USER
- `docs/sprints_knowledge/S2-AUTHENTICATION.md`

**Validação**: irssi conecta e autentica

---

### S3 - PRIVMSG Direto (User→User)
**Tempo**: 1.5h | **Blocker**: ✗ NO

Mensagens privadas entre usuários.

**Saída**:
- Handler PRIVMSG <nick>
- Roteamento por nickname
- Erros (ERR_NOSUCHNICK)

**Validação**: 2 clientes com `/msg user mensagem`

---

### S4 - Canais (JOIN/PART/QUIT + Broadcast)
**Tempo**: 2.5h | **Blocker**: ✗ NO

Infraestrutura de canais e broadcast de mensagens.

**Saída**:
- Classe Channel
- Handlers JOIN/PART/QUIT
- Broadcast (PRIVMSG #canal)
- `docs/sprints_knowledge/S4-CHANNELS-DESIGN.md`

**Validação**: Múltiplos clientes em #canal recebem mensagens

---

### S5 - Operadores & Moderação
**Tempo**: 3h | **Blocker**: ✗ NO

5 Comandos obrigatórios de operador: KICK, INVITE, TOPIC, MODE (com 5 submodos).

**Saída**:
- Sistema de permissões
- Handlers: KICK, INVITE, TOPIC
- MODE completo (+i, +t, +k, +o, +l)
- `docs/sprints_knowledge/S5-OPERATORS-DESIGN.md`

**Validação**: irssi - op executa comandos, user regular recebe ERR_CHANOPRIVSNEEDED

---

### S6 - Robustez & Entrega Final
**Tempo**: 1.5h | **Blocker**: ✗ NO

Estabilidade, sinais, testes de stress, validação final.

**Saída**:
- SIGINT handler (Ctrl+C limpeza)
- Valgrind sem leaks
- Testes de flood
- `docs/sprints_knowledge/S6-ROBUSTNESS.md`

**Validação**: irssi fluxo completo, valgrind clean

---

## 📁 Estrutura de Arquivos

```
.github/docs/
├── copilot-instructions.md       ← Instruções com justificativas
├── SPRINT_TRACKING.md            ← Status em tempo real
├── functional-requirements.md    ← Requisitos completos
├── irssi-testing-guide.md        ← Guia de testes
├── bircd-reference.md            ← Arquitetura referência
└── ...

docs/sprints_knowledge/
├── S0-BUG-ANALYSIS.md           ← Agente S0 cria
├── S1-PARSER-DESIGN.md          ← Agente S1 cria
├── S2-AUTHENTICATION.md         ← Agente S2 cria
├── S4-CHANNELS-DESIGN.md        ← Agente S4 cria
└── S5-OPERATORS-DESIGN.md       ← Agente S5 cria

test/
├── S0-reproduce-bug.sh          ← Agente S0 cria
├── S0-acceptance.sh             ← Agente S0 cria
├── S1-parser-validation.sh      ← Agente S1 cria
├── S1-acceptance.sh             ← Agente S1 cria
├── S2-acceptance.sh             ← Agente S2 cria
├── ...
```

---

## 🤖 Workflow para Agentes

### Antes de Começar

1. Leia `.github/copilot-instructions.md` (instruções + justificativas)
2. Leia `.github/docs/SPRINT_TRACKING.md` (seu sprint)
3. Leia `docs/sprints_knowledge/S{n-1}-*.md` (conhecimento do sprint anterior)
4. Leia `.github/docs/functional-requirements.md` (requisitos)

### Durante Execução

1. Implemente code com **docstrings apenas** (sem poluição)
2. Compile com `-Wall -Wextra -Werror -std=c++98`
3. Crie `test/Sn-acceptance.sh` (script testável)
4. Execute seu próprio teste antes de finalizar
5. Crie `docs/sprints_knowledge/Sn-*.md` (documentação para próximo agente)

### Ao Finalizar

1. Atualize `.github/docs/SPRINT_TRACKING.md` (marque ✓ COMPLETED)
2. Garanta que teste retorna 0 (sucesso)
3. Pronto para próximo agente

---

## ✅ Critérios de Aceitação

Cada sprint tem seus próprios critérios (ver SPRINT_TRACKING.md):

**Exemplo S1**:
- ✓ Compilação com flags corretas
- ✓ Bug Ctrl+D fixado (nenhum cliente bloqueia outro)
- ✓ Parser funciona (NICK, PRIVMSG, etc)
- ✓ Sem memory leaks
- ✓ test/S1-acceptance.sh retorna 0

---

## 🧪 Tipos de Testes

Cada sprint usa diferentes tipos (justificados em copilot-instructions.md):

1. **Aceitação** - Comportamento implementado?
2. **Regressão** - Sprints anteriores ainda funcionam?
3. **Integração** - Fluxo completo?
4. **Estabilidade** - Múltiplos clientes, flood?
5. **Memória** - Valgrind sem leaks?
6. **Protocolo** - RFC 1459 compliance (irssi)?
7. **Permissões** - Segurança (user não é op)?

---

## 📚 Referências

- **Instruções**: `.github/copilot-instructions.md`
- **Tracking**: `.github/docs/SPRINT_TRACKING.md`
- **Requisitos**: `.github/docs/functional-requirements.md`
- **Arquitetura**: `/docs/architectural-design.md`
- **Testing**: `.github/docs/irssi-testing-guide.md`
- **Referência C**: `.github/docs/bircd-reference.md`

---

## ⏱️ Timeline

| S | Task | Est. | Status |
|---|------|------|--------|
| 0 | Investigação bug | 30min | ⬜ |
| 1 | Parser + Fix | 2-3h | ⬜ |
| 2 | Autenticação | 2h | ⬜ |
| 3 | PRIVMSG direto | 1.5h | ⬜ |
| 4 | Canais | 2.5h | ⬜ |
| 5 | Operadores | 3h | ⬜ |
| 6 | Robustez | 1.5h | ⬜ |
| **Total** | | **~12.5h** | - |

---

## 🚀 Como Começar

### Para Você (Orquestrador):
1. Leia este ROADMAP.md
2. Leia `.github/docs/SPRINT_TRACKING.md`
3. Inicie sessão com Agente S0: "Execute Sprint S0 conforme ROADMAP.md"

### Para Agente:
1. Leia `ROADMAP.md` (este arquivo)
2. Leia `.github/copilot-instructions.md` (instruções + justificativas)
3. Leia `.github/docs/SPRINT_TRACKING.md` - sua seção de sprint
4. Leia referências recomendadas
5. Execute sua sprint

---

**Pronto para começar? Inicie Agente S0! 🎯**
