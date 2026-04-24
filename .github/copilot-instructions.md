# ft_irc - Copilot Instructions

## � DOCUMENTAÇÃO: Para Agentes vs Para Humanos

**⚠️ LEITURA OBRIGATÓRIA ANTES DE COMEÇAR QUALQUER SPRINT**

### `.github/docs/` - **Para Agentes de IA** (NÃO VERSIONADO)
Este diretório contém:
- Instruções, requisitos, referências técnicas
- **NÃO sobe no git** (descartado no versionamento)
- Atualizado pela equipe de desenvolvimento
- Contém "O QUÊ" fazer, "COMO" fazer, "QUANDO" fazer

**Docs críticas aqui:**
- `AGENTS_KNOWLEDGE_INDEX.md` ← **COMECE AQUI** (bússola para agentes)
- `copilot-instructions.md` (você está lendo)
- `SPRINT_TRACKING.md` (status em tempo real)
- `functional-requirements.md` (specs completos)
- `bircd-reference.md` (referência arquitetural)

### `docs/sprints_knowledge/` - **Para Humanos** (VERSIONADO)
Este diretório contém:
- Decisões, aprendizados, edge cases tratados
- **Sobe no git** (histórico permanente)
- Criado por Agentes após cada sprint
- Contém "POR QUÊ", design decisions, exemplos funcionais

**Docs críticas aqui (ler antes de sua sprint):**
- `S0-BUG-ANALYSIS.md` ← Ler se vai fazer S1+
- `S1-PARSER-DESIGN.md` ← Ler se vai fazer S2+
- `S1-PARSER-REVISION.md` ← Validação dos testes
- `architectural-design.md` ← Padrão Reactor explicado

### 🗺️ Ordem de Leitura Para Agentes
1. `.github/docs/AGENTS_KNOWLEDGE_INDEX.md` ← Você está aqui AGORA
2. Este arquivo (copilot-instructions.md) ← Seção do seu sprint
3. `.github/docs/SPRINT_TRACKING.md` ← Status atual
4. `docs/sprints_knowledge/S{n-1}-*.md` ← Contexto do agente anterior (CRÍTICO!)
5. `.github/docs/functional-requirements.md` ← Specs do projeto

---

## �🚫 PONTOS CRÍTICOS DE FALHA (NOTA 0)

1. **Mais de um `poll()`** - Apenas UM `poll()` ou equivalente (`select()`, `kqueue()`, `epoll()`) em todo o código
2. **`poll()` não chamado antes de operações** - Deve ser chamado antes de cada `accept()`, `read()/recv()`, `write()/send()`
3. **Uso de `errno` após `poll()`** - Proibido usar `errno` para disparar ações específicas após `poll()`
4. **`fcntl()` fora do padrão** - Apenas permitido: `fcntl(fd, F_SETFL, O_NONBLOCK);`. Outros usos são proibidos
5. **Segmentation fault ou crash** - O programa não deve crashar em nenhuma circunstância
6. **Compilação incorreta** - Deve compilar com `-Wall -Wextra -Werror` e `-std=c++98`
7. **Falta de regras Makefile** - Obrigatórias: `$(NAME)`, `all`, `clean`, `fclean`, `re`

---

## 🚫 PROIBIÇÕES GERAIS

### Arquitetura e I/O
- ❌ **Forking** - Uso de múltiplos processos é proibido
- ❌ **Blocking I/O** - Todo I/O deve ser não-bloqueante
- ❌ **Múltiplos `poll()`** - Apenas UM em todo o código para gerenciar TODAS as operações
- ❌ **Operações sem `poll()`** - Não tentar ler/escrever sem chamar `poll()` antes
- ❌ **Usar `errno` incorretamente** - Não usar `errno` para disparar ações após `poll()`
- ❌ **fcntl() incorreto** - Não usar variações de `fcntl()`, apenas `fcntl(fd, F_SETFL, O_NONBLOCK)`

### Compilação e Padrão
- ❌ **Não compilar com C++98** - Deve usar `-std=c++98`
- ❌ **Falta de flags obrigatórias** - Obrigatórias: `-Wall -Wextra -Werror`
- ❌ **Compilador diferente** - Usar apenas `c++`
- ❌ **Makefile incompleto** - Deve ter: `$(NAME)`, `all`, `clean`, `fclean`, `re`
- ❌ **Código em padrão C** - Priorizar C++ (ex: `<cstring>` em vez de `<string.h>`)

### Bibliotecas e Dependências
- ❌ **Bibliotecas externas** - Proibido usar bibliotecas externas e Boost
- ❌ **Libft** - Proibido usar Libft neste projeto

### Memória e Estabilidade
- ❌ **Vazamento de memória** - Toda memória alocada deve ser liberada
- ❌ **Segmentation fault** - Nenhum crash em nenhuma circunstância
- ❌ **Encerramento inesperado** - O programa deve ser estável mesmo sem memória disponível

### Rede e Conectividade
- ❌ **Não escutar em todas as interfaces** - Deve escutar em todas as interfaces de rede (0.0.0.0)
- ❌ **Não aceitar múltiplos clientes** - Deve gerenciar múltiplas conexões simultâneas sem travar
- ❌ **Processar comandos fragmentados sem agregação** - Deve agregar pacotes antes de processar

---

## ✅ REQUISITOS FUNCIONAIS OBRIGATÓRIOS

### Inicialização
- Nome do executável: `ircserv`
- Argumentos: `./ircserv <port> <password>`
- Protocolo: TCP/IP (v4 ou v6)
- Escutar em todas as interfaces de rede

### Autenticação e Identidade
- Autenticar clientes com senha fornecida
- Permitir definir `nickname`
- Permitir definir `username`

### Canais
- Comando `JOIN` para entrar em canais
- Encaminhar mensagens para todos os membros do canal
- Suportar múltiplos clientes em um mesmo canal

### Mensagens
- Comando `PRIVMSG` totalmente funcional com diferentes parâmetros
- Suportar mensagens privadas entre clientes

### Permissões
- Distinção entre usuários regulares e operadores de canal
- Usuários regulares não podem executar comandos de operador

---

## ✅ COMANDOS OBRIGATÓRIOS (OPERADOR)

Devem ser implementados, testados e funcionais:

1. **KICK** - Expulsar cliente do canal
2. **INVITE** - Convidar cliente para canal
3. **TOPIC** - Alterar ou visualizar tópico do canal
4. **MODE** - Alterar configurações do canal com submodos:
   - `i` - Invite-only (canal apenas para convidados)
   - `t` - Topic restriction (TOPIC apenas para operadores)
   - `k` - Channel key (senha do canal)
   - `o` - Operator privilege (dar/retirar operador)
   - `l` - User limit (limite de usuários)

---

## ⚠️ RESTRIÇÕES DE COMPORTAMENTO

### Manipulação de Dados
- Agregação obrigatória de pacotes fragmentados antes de processar comandos
- Reconstruir comandos completos sem afetar outras conexões

### Robustez
- Servidor deve continuar operacional se cliente encerrar abruptamente (kill)
- Servidor não deve ficar bloqueado se cliente enviar comando parcial
- Ao suspender cliente (`Ctrl-Z`) e fazer flood, servidor não deve travar
- Clientes suspensos devem processar comandos acumulados normalmente

### Testes Mínimos Obrigatórios
- Compatibilidade com `nc` (netcat)
- Compatibilidade com cliente IRC real
- Suportar múltiplas conexões simultâneas
- Teste de flood e situações extremas
- Verificação de memory leaks durante operações

---

## � DOCUMENTAÇÃO DE REFERÊNCIA

Para compreender a arquitetura e estrutura esperada do projeto, consulte:

### 📖 [`.github/docs/bircd-reference.md`](.github/docs/bircd-reference.md)
### Documentação sobre a **referência arquitetural em C** (`bircd/`). Contém:
- ✅ Visão geral do propósito da referência
- ✅ Estrutura e função de cada arquivo do `bircd/`
- ✅ Fluxo de execução detalhado (main loop)
- ✅ Estruturas de dados principais (`t_fd`, `t_env`)
- ✅ Conceitos-chave: multiplexação, I/O não-bloqueante, Reactor Pattern
- ✅ Mapeamento de conceitos: `bircd/` → `ft_irc` em C++
- ✅ Diferenças críticas e obrigações do projeto
- ✅ Como usar a referência em seu desenvolvimento
- ✅ Informação sobre cliente de referência: **irssi**

### 🏛️ [`/docs/architectural-design.md`](/docs/architectural-design.md)
Documentação da **arquitetura geral e padrão de design**. Contém:
- ✅ Explicação completa do **Reactor Pattern** (padrão arquitetural)
- ✅ Componentes: Demultiplexer, Dispatcher, Event Handlers
- ✅ Semelhança com servidores reais: Redis, Nginx, Memcached, Node.js
- ✅ Estrutura em camadas do `ft_irc`
- ✅ Fluxograma visual de execução
- ✅ Fluxo detalhado de um comando IRC
- ✅ Máquina de estados de clientes
- ✅ Desafios e soluções arquiteturais
- ✅ Próximos passos de implementação

### 🎯 [`.github/docs/development-strategy.md`](.github/docs/development-strategy.md)
Documentação da **estratégia iterativa de desenvolvimento**. Contém:
- ✅ 11 fases de desenvolvimento com tarefas específicas
- ✅ Timeline recomendada (4-6 semanas)
- ✅ Validação por requisito funcional
- ✅ Checklist de implementação completo
- ✅ Testes recomendados (diários, semanais, finais)
- ✅ Mapeamento de fases aos requisitos do projeto
- ✅ Dicas de produtividade e debugging

### 🧪 [`.github/docs/irssi-testing-guide.md`](.github/docs/irssi-testing-guide.md)
Documentação completa sobre **testes com irssi** (cliente de referência). Contém:
- ✅ Instalação e configuração do irssi
- ✅ Guia de testes estruturado em 7 fases
- ✅ Exemplos concretos de cada comando IRC
- ✅ Comandos úteis do irssi para testing
- ✅ Script de teste automatizado
- ✅ Checklist de validação com irssi
- ✅ Troubleshooting comum
- ✅ Dicas de eficiência com nc vs irssi

---

## 📋 CHECKLIST DE VALIDAÇÃO

- [ ] Apenas UM `poll()` em todo o código
- [ ] `poll()` chamado antes de cada `accept()`, `read()`, `write()`
- [ ] Nenhum uso de `errno` para disparar ações
- [ ] `fcntl()` usado apenas como: `fcntl(fd, F_SETFL, O_NONBLOCK)`
- [ ] Compilação com flags: `-Wall -Wextra -Werror -std=c++98`
- [ ] Makefile com todas as regras obrigatórias
- [ ] Nenhum crash ou segmentation fault
- [ ] Sem vazamento de memória
- [ ] Autenticação com senha funcional
- [ ] JOIN e PRIVMSG funcionais
- [ ] Todos os 5 comandos de operador implementados
- [ ] Compatibilidade com nc e cliente IRC real
- [ ] Suporte a múltiplas conexões simultâneas

---

## 🤖 INSTRUÇÕES PARA AGENTES (DESENVOLVIMENTO ITERATIVO)

### Visão Geral do Workflow

Este projeto segue um modelo de **6 Sprints com Sessões Independentes de Agentes**:

```
S0 (Blocker)  → S1 (Crítico) → S2, S3, S4 (paralelo) → S5 → S6
```

### Antes de Começar Qualquer Tarefa

**LEIA NESTA ORDEM:**
1. `.github/copilot-instructions.md` (você está aqui)
2. `.github/docs/SPRINT_TRACKING.md` (status geral do projeto)
3. `docs/sprints_knowledge/S{n-1}-*.md` (conhecimento do sprint anterior - CRÍTICO!)
4. `.github/docs/functional-requirements.md` (requisitos completos)

### Estrutura de Sprints

| Sprint | Objetivo | Blocker | Entrada | Saída Principal |
|--------|----------|---------|---------|-----------------|
| **S0** | Investigar bug Ctrl+D | ✅ SIM | Código atual | `docs/sprints_knowledge/S0-BUG-ANALYSIS.md` |
| **S1** | Parser IRC + Fix bug | ✗ | S0-BUG-ANALYSIS | `docs/sprints_knowledge/S1-PARSER-DESIGN.md` |
| **S2** | Autenticação (PASS/NICK/USER) | ✗ | S1-PARSER | `docs/sprints_knowledge/S2-AUTHENTICATION.md` |
| **S3** | PRIVMSG direto (user→user) | ✗ | S1, S2 | Código testado com irssi |
| **S4** | Canais (JOIN/PART/QUIT) | ✗ | S1, S2, S3 | `docs/sprints_knowledge/S4-CHANNELS-DESIGN.md` |
| **S5** | Operadores (KICK/INVITE/TOPIC/MODE) | ✗ | S4 | `docs/sprints_knowledge/S5-OPERATORS-DESIGN.md` |
| **S6** | Robustez (SIGINT/valgrind/irssi final) | ✗ | S5 | Entrega final validada |

### Obrigações do Agente em Cada Sprint

#### 1. Código
- ✅ **Docstrings apenas** - Sem poluição de comentários inline
- ✅ **Compilação garantida** - `c++ -Wall -Wextra -Werror -std=c++98`
- ✅ **Sem vazamentos** - Zero memory leaks (validar com valgrind)
- ✅ **Sem crashes** - Zero segmentation faults

#### 2. Testes Executáveis
- ✅ **Criar `test/S{n}-acceptance.sh`** - Script com critérios de aceitação
- ✅ **Agente EXECUTA seu próprio teste** - Antes de finalizar
- ✅ **Status claro** - Return 0 (sucesso) ou 1 (falha)

#### 3. Documentação de Conhecimento
- ✅ **Criar `docs/sprints_knowledge/S{n}-*.md`** com:
  - Design decisions e justificativas
  - Exemplos de funcionamento
  - Edge cases tratados
  - Referências para próximo agente

#### 4. Atualizar Tracking
- ✅ **Atualizar `.github/docs/SPRINT_TRACKING.md`** com:
  - Tasks completadas (✓ ou ✗)
  - Link para documentação
  - Bugs/issues encontrados
  - Dependencies resolvidas

### Validação com Cliente Real (irssi)

Sprints S2+ DEVEM validar com irssi (cliente IRC real):
```bash
irssi
/connect localhost 6667
/quote PASS senha
/nick seu_nick
/quote USER seu_user 0 * :Real Name
```

Se a resposta vier bem-formatada (RFC 1459), o protocolo está correto.

### Referências Arquiteturais

- **Padrão**: Reactor Pattern (ver `/docs/architectural-design.md`)
- **Referência C**: `bircd/` (ver `.github/docs/bircd-reference.md`)
- **Testing**: `.github/docs/irssi-testing-guide.md`
- **Estratégia**: `.github/docs/development-strategy.md`

### Pontos Críticos por Sprint

- **S0**: Root cause DEVE ser identificado e documentado antes de S1 começar
- **S1**: Parser DEVE agregar comandos fragmentados (bloqueador de estabilidade)
- **S2**: Estados de cliente DEVEM ser bem definidos (base para S3-S5)
- **S4**: Channel broadcast DEVE notificar todos os membros (não skipped)
- **S5**: Permissões DEVEM ser verificadas ANTES de executar comando (segurança)
- **S6**: Valgrind DEVE passar sem leaks (stabilidade final)

---

## 🧪 ESTRATÉGIA DE TESTES - JUSTIFICATIVAS

### Por Que Cada Teste Existe

#### 1. **Testes de Aceitação (Funcionalidade)**
**Arquivo**: `test/S{n}-acceptance.sh`

**Justificativa**: 
- Verificam se o comportamento esperado foi implementado
- Executáveis e repetíveis (não dependem de input manual)
- Agente VALIDA seu próprio trabalho
- Bloqueiam S{n+1} se falharem

**Exemplo (S1)**:
```bash
# C1: Compilação
g++ -Wall -Wextra -Werror -std=c++98 -o ircserv *.cpp || exit 1
# Justificativa: Requisito crítico de falha - sem compilação = nota 0

# C2: Bug Ctrl+D fixado
bash test/S1-bug-fix-validation.sh | grep -q "BUG FIXADO" || exit 1
# Justificativa: S0 identificou o bug - S1 DEVE corrigi-lo antes de avançar

# C3: Parser funciona
echo -e "NICK alice\r\n" | nc localhost 6667 | grep -q "NICK" || exit 1
# Justificativa: Parser é a base para S2-S6, DEVE estar pronto
```

#### 2. **Testes de Regressão (Não Quebrar)**
**Justificativa**:
- Garante que S{n} não quebrou o que S{n-1} fez
- Protege contra efeitos colaterais
- Valgrind deve rodar em CADA sprint

**Exemplo (S2)**:
```bash
# Garantir que S1 (Parser) ainda funciona
bash test/S1-acceptance.sh || exit 1
# Justificativa: S2 não deve quebrar Parser, mesmo adicionando autenticação
```

#### 3. **Testes de Integração (Fluxo Completo)**
**Justificativa**:
- Validam fluxo de usuário real (PASS → NICK → USER → READY)
- Testam interação entre componentes
- Simulam cenário real com irssi/nc

**Exemplo (S2 - Integração)**:
```bash
# Fluxo completo de autenticação
echo -e "PASS test123\r\nNICK alice\r\nUSER alice 0 * :Alice\r\n" | nc localhost 6667 | grep -q "001"
# Justificativa: Validar que todo o handshake funciona em sequência
```

#### 4. **Testes de Estabilidade (Robustez)**
**Justificativa**:
- Verificam se servidor não trava em situações extremas
- Validam conformidade com requisitos "sem crash"
- Executam em S6

**Exemplos**:
```bash
# Múltiplos clientes simultâneos
for i in {1..10}; do nc localhost 6667 &; done
wait
# Justificativa: Requisito "suportar múltiplas conexões sem travar"

# Cliente envia parcial e desconecta
(echo -n "NICK"; sleep 10) | nc localhost 6667 &
kill $!
# Justificativa: Servidor não deve ficar bloqueado com comando incompleto

# Flood de mensagens
for i in {1..100}; do echo -e "PRIVMSG #ch :msg $i\r\n"; done | nc localhost 6667
# Justificativa: Requisito "teste de flood e situações extremas"
```

#### 5. **Testes de Memória (valgrind)**
**Justificativa**:
- Detectam memory leaks ENQUANTO desenvolvem
- Validam cleanup correto (delete de Clients, Channels)
- Rodados em CADA sprint (cumulativamente)

**Exemplo**:
```bash
# valgrind em operação normal
valgrind --leak-check=full ./ircserv 6667 test123 &
PID=$!
sleep 2
# Simular 3 clientes conectando/desconectando
for i in {1..3}; do echo "" | nc localhost 6667; done
kill $PID
# Justificativa: RFC 1459 permite múltiplas conexões - cada uma deve limpar perfeitamente
```

#### 6. **Testes de Protocolo (RFC 1459 Compliance)**
**Justificativa**:
- IRC tem formato específico de respostas (ex: `:nick!user@host COMMAND param :trailing`)
- Incompatibilidade quebra clientes reais (irssi, WeeChat)
- Testados com irssi em S2+

**Exemplo (S2)**:
```bash
# Verificar formato de resposta RPL_WELCOME
echo -e "PASS test123\r\nNICK alice\r\nUSER alice 0 * :Alice\r\n" | nc localhost 6667 | grep -q "001"
# Justificativa: RFC 1459 exige código numérico 001 - irssi não reconhece sem isso
```

#### 7. **Testes de Permissões (Segurança)**
**Justificativa**:
- Validam que usuários regulares não podem fazer operações de admin
- Testados em S5 (quando permissões são implementadas)
- Evitam escalação de privilégios

**Exemplo (S5)**:
```bash
# User regular não pode fazer KICK
# alice (operadora) pode: /quote KICK #ch bob ✓
# bob (regular) não pode: /quote KICK #ch alice → ERR_CHANOPRIVSNEEDED ✓
# Justificativa: Requisito "Distinção entre usuários regulares e operadores"
```

### Mapa de Testes por Sprint

| Sprint | Tipo | Foco | Justificativa |
|--------|------|------|---------------|
| **S0** | Análise | Root cause | Bloqueia S1 |
| **S1** | Aceitação | Compilação + Parser | Base para tudo |
| **S1** | Regressão | - | N/A (primeiro) |
| **S2** | Aceitação | Autenticação | Handshake RFC |
| **S2** | Integração | PASS→NICK→USER→READY | Fluxo completo |
| **S3** | Aceitação | PRIVMSG user→user | Roteamento |
| **S4** | Aceitação | JOIN + broadcast | Canal funcional |
| **S4** | Integração | Multi-user no canal | Cenário real |
| **S5** | Aceitação | Permissões | KICK/INVITE/TOPIC/MODE |
| **S5** | Segurança | User regular vs op | Não escalação |
| **S6** | Regressão | Todos anteriores | Nada quebrou |
| **S6** | Estabilidade | Múltiplos clientes | Sem trava |
| **S6** | Memória | Valgrind completo | Zero leaks |
| **S6** | Protocolo | irssi final | RFC 1459 OK |

### Guia: Como Estruturar Output de Sprint

Ao finalizar Sprint Sn, agente deve fornecer:

```
✅ SPRINT SN COMPLETO

ARQUIVOS CRIADOS/MODIFICADOS:
├── Server.cpp / Client.cpp / *.hpp
│   └── Docstrings apenas (sem poluição)
├── test/Sn-acceptance.sh
│   └── Script testável, retorna 0/1
├── docs/sprints_knowledge/Sn-*.md
│   └── Design decisions, exemplos, edge cases
└── .github/docs/SPRINT_TRACKING.md
    └── Status: ✓ Pronto para S{n+1}

TESTES EXECUTADOS:
✓ Compilação (flags corretas)
✓ Aceitação (S{n}-acceptance.sh)
✓ Regressão (Sprint anterior não quebrou)
✓ Valgrind (sem leaks)
✓ irssi (S2+, validação com cliente real)

STATUS: ✅ PRONTO PARA PRÓXIMO AGENTE
```
