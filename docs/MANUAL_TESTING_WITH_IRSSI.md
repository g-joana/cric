# Manual Testing Guide with irssi - ft_irc

## 📖 Overview

Este documento fornece um **guia prático de testes manuais** usando irssi (cliente IRC real) para validar as features implementadas do servidor `ft_irc`.

**Referência completa**: Ver [`.github/docs/irssi-testing-guide.md`](../.github/docs/irssi-testing-guide.md) para documentação detalhada.

---

## ✅ O Que Pode Ser Testado Agora

**Status Geral do Projeto**: Sprints **S0**, **S1** e **S2** ✅ COMPLETOS

| Feature | Sprint | Status | Pode Testar? |
|---------|--------|--------|--------------|
| Conectividade TCP | S0/S1 | ✅ Completo | ✅ SIM |
| Parser (agregação de pacotes) | S1 | ✅ Completo | ✅ SIM |
| Comando PASS | S2 | ✅ Completo | ✅ SIM |
| Comando NICK | S2 | ✅ Completo | ✅ SIM |
| Comando USER | S2 | ✅ Completo | ✅ SIM |
| RPL_WELCOME (001) | S2 | ✅ Completo | ✅ SIM |
| State Machine (INIT→REGISTERED) | S2 | ✅ Completo | ✅ SIM |
| **PRIVMSG (user→user)** | **S3** | ⏳ Planejado | ❌ Não |
| **JOIN/PART/QUIT (canais)** | **S4** | ⏳ Planejado | ❌ Não |
| **Operadores (KICK/INVITE/TOPIC/MODE)** | **S5** | ⏳ Planejado | ❌ Não |

---

## 🚀 Configuração & Start

### 1. Compilar o Servidor

```bash
cd /home/scr1b3s/cric
make clean
make
```

**Esperado**: Compilação sem erros, executável `ircserv`

```bash
c++ -Wall -Wextra -Werror -std=c++98 -c Server.cpp
c++ -Wall -Wextra -Werror -std=c++98 -c Client.cpp
c++ -Wall -Wextra -Werror -std=c++98 -c CommandParser.cpp
c++ -Wall -Wextra -Werror -std=c++98 -c main.cpp
c++ -o ircserv Server.o Client.o CommandParser.o main.o
```

### 2. Iniciar o Servidor

**Terminal 1**:
```bash
./ircserv 6667 senha123
```

**Output esperado**: Servidor aguardando conexões (sem erro, sem crash)

### 3. Instalar irssi (se não tiver)

```bash
# Linux (Debian/Ubuntu)
sudo apt update && sudo apt install irssi

# Linux (Fedora/RHEL)
sudo dnf install irssi

# macOS
brew install irssi

# Verificar
irssi --version
```

### 4. Abrir irssi em Outro Terminal

**Terminal 2**:
```bash
irssi
```

Você verá a interface do irssi com um prompt.

---

## 🧪 Testes Manuais - Casos de Uso

### TESTE 1: Conectividade Básica

**Objetivo**: Validar que o servidor aceita conexão TCP

**Passos**:
```bash
# Terminal 2 (dentro do irssi)
/connect localhost 6667
```

**Esperado na tela**:
```
* Looking up localhost...
* Connecting to localhost [127.0.0.1] port 6667...
* Connection established
```

**Status**: ✅ Se conectar sem erros, teste passa

---

### TESTE 2: Autenticação - Comando PASS

**Objetivo**: Validar senha correta/incorreta

#### 2a. Senha Correta

**Terminal 2**:
```bash
# Após conectar
/quote PASS senha123
```

**Esperado**: Nenhuma mensagem de erro (silent OK)

#### 2b. Senha Incorreta

**Terminal 2**:
```bash
/quote PASS senhaErrada
```

**Esperado**: Mensagem de erro
```
* 464 * :Password incorrect
```

**Status**: ✅ Se passar e falhar nos casos corretos, teste passa

---

### TESTE 3: Comando NICK

**Objetivo**: Validar nickname válido, duplicado, vazio

#### 3a. NICK Válido

**Terminal 2**:
```bash
/quote PASS senha123
/nick alice
```

**Esperado**: Sem erro, nickname muda para `alice`

#### 3b. NICK Duplicado

**Terminal 2** (Cliente 1):
```bash
/quote PASS senha123
/nick bob
```

**Terminal 3** (Abra outro irssi):
```bash
irssi
/connect localhost 6667
/quote PASS senha123
/nick bob
```

**Esperado no Cliente 2**:
```
* 433 * bob :Nickname is already in use
```

#### 3c. NICK Vazio

**Terminal 2**:
```bash
/quote NICK
```

**Esperado**: Erro (comando incompleto)
```
* 431 * :No nickname given
```

**Status**: ✅ Se todos os casos behave correctly, teste passa

---

### TESTE 4: Comando USER

**Objetivo**: Validar captura de username e realname

#### 4a. USER Válido

**Terminal 2**:
```bash
/quote PASS senha123
/nick charlie
/quote USER charlie 0 * :Charlie Real Name
```

**Esperado**: Comando aceito

#### 4b. USER Incompleto

**Terminal 2**:
```bash
/quote PASS senha123
/quote USER charlie
```

**Esperado**:
```
* 461 * USER :Not enough parameters
```

**Status**: ✅ Se passar e falhar nos casos corretos, teste passa

---

### TESTE 5: State Machine - Handshake Completo

**Objetivo**: Validar fluxo PASS → NICK → USER → REGISTERED

#### 5a. Ordem Correta (PASS → NICK → USER)

**Terminal 2**:
```bash
/connect localhost 6667
/quote PASS senha123
/nick diana
/quote USER diana 0 * :Diana Smith
```

**Esperado**: Welcome message
```
* 001 diana :Welcome to ft_irc diana!diana@localhost
```

#### 5b. NICK Antes de PASS

**Terminal 2**:
```bash
/connect localhost 6667
/nick evan
```

**Esperado**: Erro ou aceita parcialmente mas não registra até PASS

#### 5c. Múltiplos Clientes Simultâneos

**Terminal 2** (Cliente 1):
```bash
irssi
/connect localhost 6667
/quote PASS senha123
/nick frank
/quote USER frank 0 * :Frank
```

**Terminal 3** (Cliente 2):
```bash
irssi
/connect localhost 6667
/quote PASS senha123
/nick grace
/quote USER grace 0 * :Grace
```

**Esperado**: 
- Ambos recebem RPL_WELCOME
- Nenhum bloqueia o outro
- Nenhum timeout

**Status**: ✅ Se handshake completa para ambos, teste passa

---

### TESTE 6: Robustez - Comandos Fragmentados

**Objetivo**: Validar agregação de pacotes (parser S1)

#### Via netcat (Terminal):

**Terminal 3**:
```bash
# Enviar PASS em 2 partes
(echo -n "PAS"; sleep 0.5; echo "S senha123") | nc localhost 6667 &
wait
```

**Esperado**: Servidor agrega e processa (não trata como 2 comandos diferentes)

**Status**: ✅ Se processar como 1 comando, teste passa

---

### TESTE 7: Robustez - Cliente Desconectando Abruptamente

**Objetivo**: Validar que servidor não trava quando cliente desconecta

#### 7a. Ctrl+D em irssi

**Terminal 2**:
```bash
irssi
/connect localhost 6667
/quote PASS senha123
/nick hank
# Pressione Ctrl+D para desconectar
```

**Terminal 3** (outro irssi durante desconexão):
```bash
irssi
/connect localhost 6667
# Esperado: Conecta normalmente (sem bloqueio)
```

**Status**: ✅ Se Terminal 3 conecta sem delay, teste passa (bug S0 foi fixado)

#### 7b. Matar Processo

**Terminal 3** (durante conexão ativa):
```bash
# Descobrir PID do nc
ps aux | grep irssi

# Matar processo
kill <PID>
```

**Esperado**: Servidor limpa conexão e continua aceitando outras

**Status**: ✅ Se servidor não trava, teste passa

---

### TESTE 8: Validação de Protocolos RFC

**Objetivo**: Validar que respostas seguem RFC 1459

#### 8a. Formato de RPL_WELCOME

**Terminal 2**:
```bash
/quote PASS senha123
/nick ivan
/quote USER ivan 0 * :Ivan
```

**Esperado** (na janela de log/output):
```
:ft_irc 001 ivan :Welcome to ft_irc ivan!ivan@localhost
```

**Formato esperado**: `:server CODE nick :message`

**Status**: ✅ Se seguir formato RFC, teste passa

---

## 📊 Checklist de Testes Manuais

```
[ ] T1: Conectividade básica (TCP aceita)
[ ] T2: PASS correto funciona
[ ] T2b: PASS errado rejeita (464)
[ ] T3: NICK válido aceito
[ ] T3b: NICK duplicado rejeitado (433)
[ ] T3c: NICK vazio rejeitado (431)
[ ] T4: USER com params válidos
[ ] T4b: USER sem params rejeita (461)
[ ] T5: Handshake PASS→NICK→USER→WELCOME
[ ] T5b: Múltiplos clientes sem bloqueio
[ ] T6: Comandos fragmentados agregados
[ ] T7: Cliente desconecta (Ctrl+D) não bloqueia outros
[ ] T7b: Kill de processo não trava server
[ ] T8: Formato RFC 1459 das respostas
```

**Resultado**: 14/14 testes = ✅ **S0/S1/S2 Validado**

---

## 🔧 Troubleshooting

### Problema: "Connection refused"

**Causa**: Servidor não está rodando

**Solução**:
```bash
# Terminal 1 - Verificar se ircserv está rodando
ps aux | grep ircserv

# Se não, iniciar
./ircserv 6667 senha123
```

### Problema: "Compilation failed"

**Causa**: Flags ou dependências incorretas

**Solução**:
```bash
make distclean
make clean
make
```

### Problema: "Timeout connecting"

**Causa**: Servidor pode estar em estado inválido

**Solução**:
```bash
# Matar servidor
pkill -f ircserv

# Remescar e reiniciar
./ircserv 6667 senha123
```

### Problema: Múltiplos clientes travam

**Causa**: Bug S0 ainda não foi fixado, ou regressão

**Solução**:
```bash
# Verificar status de S1
bash test/S1-bug-fix-validation.sh
```

**Esperado**: `✓ Bug fixado`

---

## 📚 Documentação Relacionada

- [`.github/docs/irssi-testing-guide.md`](../.github/docs/irssi-testing-guide.md) - Guia completo (6 fases de teste)
- [`docs/sprints_knowledge/S2-AUTHENTICATION.md`](sprints_knowledge/S2-AUTHENTICATION.md) - Design de autenticação
- [`docs/sprints_knowledge/S1-PARSER-DESIGN.md`](sprints_knowledge/S1-PARSER-DESIGN.md) - Design do parser
- [`docs/sprints_knowledge/S0-BUG-ANALYSIS.md`](sprints_knowledge/S0-BUG-ANALYSIS.md) - Análise do bug Ctrl+D

---

## 🎯 Próximos Passos

Quando **S3 estiver completo**, este guia será atualizado com:

```
[ ] TESTE 9: PRIVMSG entre clientes
[ ] TESTE 10: PRIVMSG para nick inexistente (404)
[ ] TESTE 11: Múltiplos PRIVMSG simultâneos
```

---

## 📝 Notas

- **irssi vs nc**: irssi é melhor para testes manuais interativos, `nc` para automação
- **Protocolo**: `ft_irc` implementa **RFC 1459** (padrão IRC)
- **Segurança**: Testes locais apenas (`localhost:6667`)
- **Performance**: Sem otimizações em testes manuais (foco em funcionalidade)

---

## 🤝 Contribuindo

Se encontrar issues nos testes manuais:

1. Documentar no terminal (copiar output)
2. Checar correspondência com [`test/S{n}-acceptance.sh`](../test/)
3. Abrir issue ou atualizar SPRINT_TRACKING.md

**Última atualização**: 2026-04-24  
**Status**: ✅ Pronto para S0/S1/S2  
**Próximo**: Aguardando S3 (PRIVMSG)
