# cric - ft_irc

**Servidor IRC (Internet Relay Chat)** implementado em **C++98** com arquitetura não-bloqueante usando `poll()`.

Projeto desenvolvimento iterativo com **Sprints de Agentes Independentes** (S0-S6).

---

## 🎯 Objetivo

Implementar um servidor IRC totalmente funcional em C++ que suporte:
- ✅ Autenticação com senha (`PASS`, `NICK`, `USER`)
- ✅ Mensagens privadas (`PRIVMSG` user→user)
- ✅ Canais (JOIN, PART, QUIT) com broadcast
- ✅ Comandos de moderação (KICK, INVITE, TOPIC, MODE)
- ✅ Compatibilidade com clientes IRC reais (irssi, WeeChat)

---

## 📋 Início Rápido

### Compilação

```bash
make        # Compila o servidor (gera ircserv)
make clean  # Remove arquivos .o
make fclean # Remove binário
make distclean # Remove tudo (debug, logs)
```

### Execução

```bash
./ircserv <port> <password>

# Exemplo:
./ircserv 6667 secretpassword
```

### Conexão (com nc ou irssi)

```bash
# Via netcat
nc localhost 6667

# Via irssi
irssi
/connect localhost 6667
/quote PASS secretpassword
/nick your_nick
/quote USER your_user 0 * :Your Name
```

---

## 📁 Estrutura do Projeto

```
cric/
├── .github/docs/              ← Documentação para Agentes (NÃO versionada)
│   ├── AGENTS_KNOWLEDGE_INDEX.md    (Bússola para agentes - COMECE AQUI)
│   ├── copilot-instructions.md      (Instruções + requisitos críticos)
│   ├── SPRINT_TRACKING.md           (Status em tempo real)
│   ├── functional-requirements.md   (Specs completos do projeto)
│   ├── bircd-reference.md           (Arquitetura C de referência)
│   ├── development-strategy.md      (Roadmap 11 fases)
│   ├── irssi-testing-guide.md       (Guia de testes com irssi)
│   └── project_rubric.md            (Critérios de avaliação)
│
├── docs/                      ← Documentação para Humanos (VERSIONADA)
│   ├── architectural-design.md      (Padrão Reactor, design C++)
│   └── sprints_knowledge/           (Conhecimento acumulado por sprint)
│       ├── S0-BUG-ANALYSIS.md       (Análise do bug Ctrl+D)
│       ├── S1-PARSER-DESIGN.md      (Design do parser)
│       ├── S1-PARSER-REVISION.md    (Validação de testes)
│       └── ... (S2+)
│
├── test/                      ← Scripts de teste
│   ├── S0-acceptance.sh
│   ├── S0-aggressive-test.sh
│   ├── S0-reproduce-bug.sh
│   ├── S1-acceptance.sh
│   ├── S1-bug-fix-validation.sh
│   ├── S1-parser-integration-test.sh
│   ├── S1-parser-validation.sh
│   └── run-parser-unit-tests.sh
│
├── bircd/                     ← Referência arquitetural (C)
│
├── *.cpp / *.hpp              ← Código-fonte (C++98)
│   ├── main.cpp
│   ├── Server.cpp / Server.hpp
│   ├── Client.cpp / Client.hpp
│   └── CommandParser.cpp / CommandParser.hpp
│
├── Makefile                   ← Build system
├── .gitignore
└── README.md (você está aqui)
```

---

## 📚 Documentação

### Para Humanos (GitHub - VERSIONADA)
Ler em `docs/sprints_knowledge/` para entender decisões técnicas:

- **[S0-BUG-ANALYSIS.md](docs/sprints_knowledge/S0-BUG-ANALYSIS.md)** - Identificação de bugs críticos (poll/erase)
- **[S1-PARSER-DESIGN.md](docs/sprints_knowledge/S1-PARSER-DESIGN.md)** - Design do parser robusto para comandos fragmentados
- **[S1-PARSER-REVISION.md](docs/sprints_knowledge/S1-PARSER-REVISION.md)** - Testes 8/8 unitários, edge cases tratados
- **[architectural-design.md](docs/architectural-design.md)** - Explicação do padrão Reactor (como o servidor funciona)

### Para Agentes (NÃO VERSIONADA)
Ler em `.github/docs/` para instruções de desenvolvimento:

- **[AGENTS_KNOWLEDGE_INDEX.md](.github/docs/AGENTS_KNOWLEDGE_INDEX.md)** ← **Agentes começam aqui**
- **[functional-requirements.md](.github/docs/functional-requirements.md)** - Requisitos completos
- **[bircd-reference.md](.github/docs/bircd-reference.md)** - Arquitetura da referência (bircd/)
- **[irssi-testing-guide.md](.github/docs/irssi-testing-guide.md)** - Como testar com cliente real

---

## 🧪 Testes

O projeto usa **testes executáveis por sprint**:

```bash
# S1 - Parser (agregar pacotes fragmentados)
bash test/S1-acceptance.sh      # Status: ✓ Passou

# Unitários do parser
bash test/run-parser-unit-tests.sh

# Validação de bug fixado
bash test/S1-bug-fix-validation.sh

# Limpeza pós-teste
make distclean
```

**Tipos de testes inclusos**:
- ✅ **Aceitação**: Funcionalidade obrigatória
- ✅ **Unitários**: Isolamento de componentes (CommandParser)
- ✅ **Integração**: Fluxo de cliente+servidor
- ✅ **Regressão**: Validar que versão anterior não quebrou
- ✅ **Memória**: `valgrind` para detectar leaks

---

## 🚀 Status dos Sprints

Metodologia: **6 Sprints Independentes** com entrega iterativa

| Sprint | Objetivo | Status | Documento |
|--------|----------|--------|-----------|
| **S0** | Investigar bug Ctrl+D | ✅ Completo | [S0-BUG-ANALYSIS.md](docs/sprints_knowledge/S0-BUG-ANALYSIS.md) |
| **S1** | Parser + Bugs S0 | ✅ Completo | [S1-PARSER-DESIGN.md](docs/sprints_knowledge/S1-PARSER-DESIGN.md) |
| **S2** | Autenticação (PASS/NICK/USER) | ⏳ Planejado | - |
| **S3** | PRIVMSG user→user | ⏳ Planejado | - |
| **S4** | Canais (JOIN/PART/QUIT) | ⏳ Planejado | - |
| **S5** | Operadores (KICK/INVITE/TOPIC/MODE) | ⏳ Planejado | - |
| **S6** | Robustez (SIGINT/valgrind final) | ⏳ Planejado | - |

---

## 📋 Tarefas Em Progresso

Itens críticos já completos:
- ✅ **S0**: Bug Ctrl+D identificado (2 bugs em poll/erase)
- ✅ **S1**: CommandParser com agregação de pacotes
- ✅ **S1**: Testes unitários 8/8 passando

Próximos itens (S2-S6):
- ⏳ Handlers PASS, NICK, USER (S2)
- ⏳ Estados de cliente (INIT → AUTH → REGISTERED) (S2)
- ⏳ Roteamento PRIVMSG por nickname (S3)
- ⏳ Classe Channel com broadcast (S4)
- ⏳ Sistema de permissões e KICK/INVITE/TOPIC/MODE (S5)

---

## ⚙️ Compilação & Flags

```makefile
CXX       = c++
CXXFLAGS  = -Wall -Wextra -Werror -std=c++98

make        # Compila com flags obrigatórias
```

**Requisitos**:
- ✅ C++98 (sem C++11+)
- ✅ `-Wall -Wextra -Werror` (sem warnings)
- ✅ Sem bibliotecas externas (sem Boost, sem Libft)

---

## 🔧 Arquitetura

**Padrão**: Reactor Pattern (não-bloqueante)

```
┌─────────────────┐
│   main()        │
└────────┬────────┘
         │
         ▼
┌─────────────────────────────┐
│  Server::run()              │
│  ┌─────────────────────┐    │
│  │ poll() [1 única]    │◄───┼─ Multiplexação: fd's monitorados
│  │ - Sockets           │    │
│  │ - Clientes          │    │
│  │ - Canais (após S4)  │    │
│  └─────────────────────┘    │
│         │                   │
│         ├─► Accept novo     │
│         ├─► Read client     │
│         ├─► Parse comando   │
│         └─► Dispatch cmd    │
└─────────────────────────────┘
```

Veja [architectural-design.md](docs/architectural-design.md) para detalhes.

---

## 🤝 Contribuir

Este projeto segue metodologia de **Sprints com Agentes Independentes**.

Se você é um **agente de IA**:
1. Ler [AGENTS_KNOWLEDGE_INDEX.md](.github/docs/AGENTS_KNOWLEDGE_INDEX.md)
2. Ler [copilot-instructions.md](.github/docs/copilot-instructions.md)
3. Começar seu sprint

Se você é um **desenvolvedor humano**:
1. Ler [docs/sprints_knowledge/](docs/sprints_knowledge/) para contexto
2. Validar requisitos em [functional-requirements.md](.github/docs/functional-requirements.md)
3. Executar testes com `bash test/S{n}-acceptance.sh`

---

## 📖 Referência

- **RFC 1459**: [Internet Relay Chat Protocol](https://tools.ietf.org/html/rfc1459)
- **bircd**: Implementação C de referência em `bircd/`
- **irssi**: Cliente IRC para testes
- **netcat**: Ferramenta para testes básicos

---

## 📝 Licença

Projeto de aprendizado. Sem restrições.
