# Sprint Tracking - ft_irc

## Status Geral do Projeto

**Timeline**: 6 Sprints independentes
**Target**: Entrega funcional com testes contínuos
**Última atualização**: 2026-04-24

---

## Sprints

### ✓ COMPLETED / ⏳ IN PROGRESS / ⬜ NOT STARTED

#### S0 - Investigação: Bug Ctrl+D
**Status**: ✅ COMPLETED (2026-04-24)
**Blocker**: ✅ YES (bloqueia S1)
**Tasks**:
- [x] S0-T1: Análise Server::run() - fluxo de desconexão
- [x] S0-T2: Reproduzir bug com script (2+ clientes)
- [x] S0-T3: Identificar root cause (poll? remoção? recv?)
- [x] S0-T4: Documentar em `docs/sprints_knowledge/S0-BUG-ANALYSIS.md`

**Expected Output**:
- `docs/sprints_knowledge/S0-BUG-ANALYSIS.md`
- `test/S0-reproduce-bug.sh` (script de reprodução)
- `test/S0-acceptance.sh` (checklist de validação)

**Knowledge Files**:
- ✅ `docs/sprints_knowledge/S0-BUG-ANALYSIS.md` - Root cause análise com 2 bugs identificados

**Criteria** (✓ all passed):
- ✓ Root cause documentado (Bug #1: modificação vetor durante iteração; Bug #2: remoção incorreta com índice)
- ✓ Bug reproduzível via script (Cliente 2 bloqueado após Cliente 1 desconectar)
- ✓ Pronto para S1 corrigir

**Evidence**:
- Teste S0-aggressive-test.sh: "✗ Cliente 2 NÃO recebeu resposta"
- FD 4 aparece desconectando 2x em logs (reutilização de FD)
- Loop principal tem bugs de modificação durante iteração

**Dependencies**: Nenhuma

---

#### S1 - Parser IRC + Bug Fix
**Status**: ✅ COMPLETED (2026-04-24)
**Blocker**: ✗ NO
**Tasks**:
- [x] S1-T1: Corrigir bug baseado em S0-BUG-ANALYSIS
- [x] S1-T2: Criar CommandParser (agregar \r\n)
- [x] S1-T3: Integrar parser em Client
- [x] S1-T4: Buffer handling (dados não-processados preservados)
- [x] S1-T5: Testes com nc (fragmentado, múltiplos)

**Expected Output**:
- ✅ Código: Server.cpp (bug fixes), CommandParser.hpp/cpp
- ✅ `docs/sprints_knowledge/S1-PARSER-DESIGN.md`
- ✅ `test/S1-bug-fix-validation.sh`
- ✅ `test/S1-parser-validation.sh`
- ✅ `test/S1-acceptance.sh`

**Knowledge Files Created**:
- ✅ `docs/sprints_knowledge/S1-PARSER-DESIGN.md` - Design completo, exemplos, casos de teste

**Criteria** (✓ all passed):
- ✓ Compilação com flags corretas (-Wall -Wextra -Werror -std=c++98)
- ✓ Bug #1 fixado: Modificação de vector durante iteração (usa _pendingConnections)
- ✓ Bug #2 fixado: Remoção sem decremento (loop manual, não incrementa após erase)
- ✓ CommandParser funciona (agrega \r\n, extrai comandos)
- ✓ Parser integrado em Client e Server
- ✓ Sem segmentation fault (timeout test passou)
- ✓ Sem memory leaks (valgrind check)
- ✓ `test/S1-acceptance.sh` retorna 0

**Evidence**:
- ✓ Compilação: `c++ -Wall -Wextra -Werror -std=c++98 *.cpp` OK
- ✓ Bug Fix Test: Múltiplos clients conectam/desconectam sem crash
- ✓ Parser Test: Comandos simples, fragmentados e múltiplos funcionam
- ✓ Acceptance: Todos 5 critérios passaram

**Arquivos Modificados**:
- Server.hpp: Adicionado _pendingConnections, _processPendingConnections()
- Server.cpp: Refatorado run() com loop manual, usa _pendingConnections
- Client.hpp: Integrado CommandParser
- Client.cpp: Delegação para CommandParser
- CommandParser.hpp: Nova classe
- CommandParser.cpp: Implementação
- Makefile: Adicionado CommandParser.cpp
- test/S1-acceptance.sh: Script de validação
- test/S1-bug-fix-validation.sh: Teste de bugs
- test/S1-parser-validation.sh: Teste de parser
- docs/sprints_knowledge/S1-PARSER-DESIGN.md: Documentação

**Dependencies**: S0 (analysis completed) ✓

**Status para S2**: ✅ PRONTO - Parser funciona, bugs fixados
- CommandParser.appendData() funciona
- CommandParser.hasCompleteCommand() e extractCommand() prontos
- Client::appendToBuffer(), hasCompleteCommand(), extractCommand() prontos
- Server aguarda comandos em Client

---

#### S2 - Autenticação (PASS/NICK/USER)
**Status**: ⬜ NOT STARTED
**Blocker**: ✗ NO
**Tasks**:
- [ ] S2-T1: Comando PASS (verificar senha)
- [ ] S2-T2: Comando NICK (validar, verificar duplicatas)
- [ ] S2-T3: Comando USER (capturar dados)
- [ ] S2-T4: ClientState (INIT → AUTH → ID → REGISTERED)
- [ ] S2-T5: RPL_WELCOME (001) após handshake
- [ ] S2-T6: Testes com irssi e nc

**Expected Output**:
- Código: Client state machine, command handlers
- `docs/sprints_knowledge/S2-AUTHENTICATION.md`
- `test/S2-acceptance.sh`

**Knowledge Files Required to Read**:
- `docs/sprints_knowledge/S1-PARSER-DESIGN.md` (como parser funciona) ✓
- `.github/docs/functional-requirements.md`
- `.github/docs/irssi-testing-guide.md` (FASE 2)

**Criteria** (Accept when ✓ all):
- ✓ Senha errada rejeita cliente
- ✓ Nick duplicado rejeitado (ERR_NICKNAMEINUSE)
- ✓ RPL_WELCOME (001) após PASS+NICK+USER
- ✓ Estados de cliente funcionam
- ✓ irssi conecta e autentica com sucesso
- ✓ `test/S2-acceptance.sh` retorna 0

**Dependencies**: S1 (parser)

---

#### S3 - PRIVMSG Direto (User→User)
**Status**: ⬜ NOT STARTED
**Blocker**: ✗ NO
**Tasks**:
- [ ] S3-T1: Criar findClientByNickname()
- [ ] S3-T2: Comando PRIVMSG <nick> :<msg>
- [ ] S3-T3: Roteamento (buscar FD destino)
- [ ] S3-T4: Tratamento de erros (ERR_NOSUCHNICK)
- [ ] S3-T5: Formato correto (`:remetente!user@host PRIVMSG dest :msg`)
- [ ] S3-T6: Testes com 2+ clientes via irssi

**Expected Output**:
- Código: PRIVMSG handler
- `test/S3-acceptance.sh`

**Knowledge Files Required to Read**:
- `docs/sprints_knowledge/S1-PARSER-DESIGN.md`
- `docs/sprints_knowledge/S2-AUTHENTICATION.md`
- `.github/docs/irssi-testing-guide.md` (Mensagens Privadas)

**Criteria** (Accept when ✓ all):
- ✓ Mensagem chega ao usuário correto
- ✓ Remetente recebe confirmação
- ✓ Usuário inexistente gera erro
- ✓ Formato IRC correto
- ✓ irssi: `/msg user mensagem` funciona
- ✓ `test/S3-acceptance.sh` retorna 0

**Dependencies**: S2 (autenticação)

---

#### S4 - Canais (JOIN/PART/QUIT + Broadcast)
**Status**: ⬜ NOT STARTED
**Blocker**: ✗ NO
**Tasks**:
- [ ] S4-T1: Classe Channel (membros, operadores, tópico)
- [ ] S4-T2: Comando JOIN (adicionar, notificar entrada)
- [ ] S4-T3: Broadcast em canais (PRIVMSG #ch)
- [ ] S4-T4: Comando PART (remover, notificar saída)
- [ ] S4-T5: Comando QUIT (encerrar + notificar canais)
- [ ] S4-T6: Testes com múltiplos clientes em canais

**Expected Output**:
- Código: Channel class, JOIN/PART/QUIT/broadcast
- `docs/sprints_knowledge/S4-CHANNELS-DESIGN.md`
- `test/S4-acceptance.sh`

**Knowledge Files Required to Read**:
- `docs/sprints_knowledge/S1-PARSER-DESIGN.md`
- `docs/sprints_knowledge/S2-AUTHENTICATION.md`
- `docs/sprints_knowledge/S3-PRIVMSG-DESIGN.md` (se criado)
- `.github/docs/irssi-testing-guide.md` (FASE 3)

**Criteria** (Accept when ✓ all):
- ✓ Cliente entra em canal, notifica outros
- ✓ Mensagem em canal chega a todos
- ✓ Notificações com formato IRC correto
- ✓ PART remove cliente
- ✓ QUIT encerra + notifica canais
- ✓ irssi: múltiplos clientes em #canal funciona
- ✓ `test/S4-acceptance.sh` retorna 0

**Dependencies**: S2 (autenticação), S3 (para roteamento de msgs)

---

#### S5 - Operadores & Moderação (KICK/INVITE/TOPIC/MODE)
**Status**: ⬜ NOT STARTED
**Blocker**: ✗ NO
**Tasks**:
- [ ] S5-T1: Sistema de permissões (op vs membro)
- [ ] S5-T2: Comando KICK (expulsar + validar permissão)
- [ ] S5-T3: Comando INVITE (convidar + notificar)
- [ ] S5-T4: Comando TOPIC (ver/mudar + permissão +t)
- [ ] S5-T5: Comando MODE com submodos:
  - [ ] +i/-i (invite-only)
  - [ ] +t/-t (topic restriction)
  - [ ] +k/-k (password)
  - [ ] +o/-o (operator)
  - [ ] +l/-l (user limit)
- [ ] S5-T6: Testes com irssi (permissões, escalação)

**Expected Output**:
- Código: Permission system, 5 command handlers, Mode logic
- `docs/sprints_knowledge/S5-OPERATORS-DESIGN.md`
- `test/S5-acceptance.sh`

**Knowledge Files Required to Read**:
- `docs/sprints_knowledge/S4-CHANNELS-DESIGN.md`
- `.github/docs/functional-requirements.md` (comandos obrigatórios)
- `.github/docs/irssi-testing-guide.md` (FASE 5-8)

**Criteria** (Accept when ✓ all):
- ✓ Apenas ops podem executar comandos restritos
- ✓ KICK remove e notifica
- ✓ INVITE envia notificação
- ✓ TOPIC com +t só permite ops
- ✓ MODE +i força invites
- ✓ MODE +k requer senha para JOIN
- ✓ User regular recebe ERR_CHANOPRIVSNEEDED
- ✓ irssi: todos 5 comandos funcionam
- ✓ `test/S5-acceptance.sh` retorna 0

**Dependencies**: S4 (canais)

---

#### S6 - Robustez & Entrega Final
**Status**: ⬜ NOT STARTED
**Blocker**: ✗ NO
**Tasks**:
- [ ] S6-T1: Tratamento SIGINT (Ctrl+C limpeza)
- [ ] S6-T2: Valgrind completo (todos sprints)
- [ ] S6-T3: Teste de flood (múltiplos msgs)
- [ ] S6-T4: Compatibilidade final irssi
- [ ] S6-T5: Checklist de requisitos vs código

**Expected Output**:
- Código: SIGINT handler, cleanup
- `docs/sprints_knowledge/S6-ROBUSTNESS.md`
- `test/S6-acceptance.sh`

**Knowledge Files Required to Read**:
- `docs/sprints_knowledge/S5-OPERATORS-DESIGN.md`
- `.github/copilot-instructions.md` (requisitos críticos)
- `.github/docs/functional-requirements.md` (validação final)

**Criteria** (Accept when ✓ all):
- ✓ Ctrl+C encerra limpo (sem segfault)
- ✓ Valgrind sem leaks
- ✓ Flood handling (100+ msgs rápido)
- ✓ irssi: fluxo completo (auth → join → msg → kick → sair)
- ✓ Todos requisitos de functional-requirements.md OK
- ✓ `test/S6-acceptance.sh` retorna 0

**Dependencies**: S5 (operadores), S1-S4 (regressão)

---

## Timeline Estimado

| Sprint | Tempo | Status |
|--------|-------|--------|
| S0 | 30min | ⬜ |
| S1 | 2-3h | ⬜ |
| S2 | 2h | ⬜ |
| S3 | 1.5h | ⬜ |
| S4 | 2.5h | ⬜ |
| S5 | 3h | ⬜ |
| S6 | 1.5h | ⬜ |
| **TOTAL** | **~12.5h** | - |

**Possibilidade de paralelismo**: Após S1, S2-S5 podem rodar em paralelo (dependências independentes)

---

## Checklist de Estrutura

- [x] `.github/copilot-instructions.md` - Instruções com justificativas de testes
- [x] `.github/docs/SPRINT_TRACKING.md` - Este arquivo
- [ ] `docs/sprints_knowledge/` - Diretório pronto (agentes preenchem)
- [ ] `test/` - Diretório pronto (agentes preenchem)
- [ ] `ROADMAP.md` - Documento de referência (opcional)

---

## Como Usar Este Arquivo

**Para Agentes**:
1. Leia a seção do seu sprint
2. Knowledge Files Required = arquivos para ler antes
3. Tasks = o que fazer
4. Criteria = testes que devem passar
5. Ao finalizar, atualize seu status para ✓ COMPLETED

**Para Você (Orquestrador)**:
1. Verifique o status geral
2. Inicie sessão de agente com sprint atual
3. Agente executa e atualiza este arquivo
4. Valide com `bash test/Sn-acceptance.sh`
5. Próximo agente lê docs do agente anterior

---

## Notas de Desenvolvimento

### Bugs Conhecidos
- S0: Bug Ctrl+D bloqueia outros clientes (será investigado)

### Decisões Arquiteturais
- UM poll() único (requisito crítico)
- C++98 (requisito crítico)
- Reactor Pattern (design pattern)
- irssi como cliente de referência (RFC 1459)

### Referências Principais
- `.github/copilot-instructions.md` - Instruções para agentes
- `.github/docs/functional-requirements.md` - Requisitos completos
- `.github/docs/bircd-reference.md` - Referência de arquitetura
- `/docs/architectural-design.md` - Design Reactor Pattern
- `.github/docs/irssi-testing-guide.md` - Guia de testes
