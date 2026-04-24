# 🤖 AGENTS KNOWLEDGE INDEX

**Este documento é EXCLUSIVAMENTE para Agentes de IA.**

Humanos devem ler `docs/sprints_knowledge/` em vez disso.

---

## 📚 Distinção Crítica: Duas Camadas de Documentação

### `.github/docs/` - **Para Agentes**
- Instruções, requisitos, referências arquiteturais
- **NÃO VERSIONADA** (não sobe no git)
- Atualizada pela equipe de desenvolvimento
- Leitura obrigatória antes de cada sprint
- Contém "O QUÊ" fazer e "COMO" fazer

### `docs/sprints_knowledge/` - **Para Humanos**
- Documentação de DECISÕES e APRENDIZADOS
- **VERSIONADA** (sobe no git, histórico permanente)
- Criada por Agentes, lida por humanos
- Contém "POR QUÊ" foram feitas certas decisões
- Referência técnica para próximas sprints

---

## 🗺️ Mapa de Leitura Por Sprint

### **Antes de Começar Qualquer Sprint**

1. **OBRIGATÓRIO**: Ler `.github/copilot-instructions.md` (este arquivo referenceia seu sprint)
2. **OBRIGATÓRIO**: Ler `.github/docs/SPRINT_TRACKING.md` (status atual)
3. **CRÍTICO**: Ler `docs/sprints_knowledge/S{n-1}-*.md` (sprint anterior - seu contexto)
4. **RECOMENDADO**: Ler `.github/docs/functional-requirements.md` (requisitos completos)

### **Ordem Exata de Documentação**

```
┌─────────────────────────────────────────────────────────────┐
│ 1. Ler copilot-instructions.md (VOCÊ ESTÁ AQUI AGORA)      │
│    ↓ Entender requisitos gerais                            │
│ 2. Ler SPRINT_TRACKING.md (status em tempo real)           │
│    ↓ Ver qual sprint você vai fazer                        │
│ 3. Ler docs/sprints_knowledge/S{n-1}-*.md (contexto)       │
│    ↓ Entender decisões do agente anterior                  │
│ 4. Ler .github/docs/functional-requirements.md (specs)     │
│    ↓ Validar requisitos específicos                        │
│ 5. Ler referências conforme necessário                      │
│    └─ .github/docs/bircd-reference.md (arquitetura C)      │
│    └─ docs/architectural-design.md (padrão design)         │
│    └─ .github/docs/irssi-testing-guide.md (testes)         │
│                                                             │
│ 🎯 CRIAR `docs/sprints_knowledge/S{n}-*.md` (sua sprint)   │
│    └─ Após completar, atualizar SPRINT_TRACKING.md         │
└─────────────────────────────────────────────────────────────┘
```

---

## 📋 Quick Reference: Arquivos Por Propósito

| Arquivo | Público | Propósito | Status |
|---------|---------|-----------|--------|
| `copilot-instructions.md` | Agentes | Instruções gerais, obrigações, pontos críticos | ✅ Versão atual |
| `SPRINT_TRACKING.md` | Agentes | Status em tempo real de cada sprint | ✅ Atualizado por agentes |
| `functional-requirements.md` | Agentes | Requisitos completos do projeto | ✅ Estável |
| `bircd-reference.md` | Agentes | Referência arquitetural em C (bircd/) | ✅ Referência |
| `development-strategy.md` | Agentes | Estratégia geral (11 fases) | ✅ Roadmap |
| `irssi-testing-guide.md` | Agentes | Como testar com cliente irssi | ✅ Guia |
| `project_rubric.md` | Agentes | Critérios de avaliação | ✅ Rubric |
| --- | --- | --- | --- |
| `architectural-design.md` | Humanos | Padrão Reactor, arquitetura C++ | 📖 Referência |
| `sprints_knowledge/S0-BUG-ANALYSIS.md` | Humanos | O que foi descoberto em S0 | ✅ Completo |
| `sprints_knowledge/S1-PARSER-DESIGN.md` | Humanos | Design do parser implementado em S1 | ✅ Completo |
| `sprints_knowledge/S1-PARSER-REVISION.md` | Humanos | Validação dos testes do parser | ✅ Completo |

---

## 🎯 Checklist: O Que Fazer Em Cada Sprint

### Antes de Começar (30 min)
- [ ] Ler este índice (AGENTS_KNOWLEDGE_INDEX.md)
- [ ] Ler copilot-instructions.md (seção do seu sprint)
- [ ] Ler SPRINT_TRACKING.md (seu status atual)
- [ ] Ler docs/sprints_knowledge/S{n-1}-*.md (contexto anterior)

### Durante o Desenvolvimento
- [ ] Criar/atualizar código com docstrings apenas
- [ ] Compilar com `-Wall -Wextra -Werror -std=c++98`
- [ ] Rodar testes enquanto desenvolve
- [ ] Validar com `make clean && make` (reset total)

### Ao Finalizar o Sprint
- [ ] Criar `test/S{n}-acceptance.sh` (executável, return 0/1)
- [ ] Criar `docs/sprints_knowledge/S{n}-*.md` (decisões + exemplos)
- [ ] Rodar `valgrind` sem leaks
- [ ] Atualizar `.github/docs/SPRINT_TRACKING.md`
- [ ] Garantir `.gitignore` deixa versionado o que deve ser

---

## ⚠️ Leituras Obrigatórias (NÃO PULE)

1. **copilot-instructions.md**:
   - Seção "🚫 PONTOS CRÍTICOS DE FALHA"
   - Seção "✅ REQUISITOS FUNCIONAIS OBRIGATÓRIOS"
   - Sua sprint específica em "Obrigações do Agente em Cada Sprint"

2. **functional-requirements.md**:
   - Seção "3. Requisitos Funcionais"
   - Seção "4. Comandos de Operador"
   - Notas críticas para avaliação

3. **SPRINT_TRACKING.md**:
   - Seu sprint atual (status, bloqueadores, dependências)

4. **docs/sprints_knowledge/S{n-1}-*.md**:
   - **CRÍTICO**: O que o agente anterior já fez
   - Decisões que afetam seu código
   - Edge cases já tratados

---

## 🔄 Workflow Resumido Por Sprint

```
S0 - BUG ANALYSIS (Blocker)
  Lê: copilot, SPRINT_TRACKING, functional-requirements
  Cria: S0-BUG-ANALYSIS.md, test/S0-acceptance.sh
  Sai: docs/sprints_knowledge/S0-BUG-ANALYSIS.md

S1 - PARSER (Crítico)
  Lê: copilot, SPRINT_TRACKING, S0-BUG-ANALYSIS, bircd-reference
  Cria: CommandParser, test/S1-acceptance.sh
  Sai: docs/sprints_knowledge/S1-PARSER-DESIGN.md

S2 - AUTENTICAÇÃO
  Lê: copilot, SPRINT_TRACKING, S1-PARSER-DESIGN, functional-requirements
  Cria: Handlers PASS/NICK/USER
  Sai: docs/sprints_knowledge/S2-AUTHENTICATION.md

S3 - PRIVMSG DIRETO
  Lê: copilot, SPRINT_TRACKING, S2-AUTHENTICATION
  Cria: Handler PRIVMSG user→user
  Sai: Código testado com irssi

S4 - CANAIS
  Lê: copilot, SPRINT_TRACKING, S2, S3
  Cria: Classe Channel, handlers JOIN/PART/QUIT
  Sai: docs/sprints_knowledge/S4-CHANNELS-DESIGN.md

S5 - OPERADORES
  Lê: copilot, SPRINT_TRACKING, S4-CHANNELS
  Cria: KICK/INVITE/TOPIC/MODE
  Sai: docs/sprints_knowledge/S5-OPERATORS-DESIGN.md

S6 - ROBUSTEZ
  Lé: copilot, SPRINT_TRACKING, S5-OPERATORS
  Cria: SIGINT handler, valgrind clean, testes finais
  Sai: Entrega final validada, S6-ROBUSTNESS.md
```

---

## 💡 Tips para Não se Perder

1. **Quando não sabe o que fazer**: Ler copilot-instructions.md seção "🤖 INSTRUÇÕES PARA AGENTES"
2. **Quando quer entender o histórico**: Ler docs/sprints_knowledge/
3. **Quando quer implementar X comando**: Ler functional-requirements.md + bircd-reference.md
4. **Quando está com bug**: Ler docs/sprints_knowledge/ de sprints anteriores (podem ter resolvido coisa parecida)
5. **Quando quer testar com cliente real**: Ler .github/docs/irssi-testing-guide.md

---

## 🎓 Resumo Para Memória

> **Regra de Ouro**: Se está desenvolvendo um Sprint, e não sabe por onde começar, comece por:
> 1. Ler `copilot-instructions.md`
> 2. Ler `docs/sprints_knowledge/S{n-1}-*.md`
> 3. Depois ler `.github/docs/functional-requirements.md`
>
> **Depois disso, você sabe o que fazer.**
