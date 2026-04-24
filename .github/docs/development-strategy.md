# Estratégia de Desenvolvimento - ft_irc com irssi

## 📋 Visão Geral da Estratégia

Este documento descreve a **roadmap de desenvolvimento** para o projeto `ft_irc`, com foco em alcançar todos os requisitos de forma iterativa, utilizando **irssi** como cliente de referência para validação.

---

## 🎯 Fases de Desenvolvimento

### FASE 1: Infraestrutura Base (Semana 1)

**Objetivo**: Estabelecer a base do servidor com multiplexação de I/O.

#### Tarefas
- [ ] Implementar socket servidor
- [ ] Implementar `poll()` único e não-bloqueante
- [ ] Implementar aceitação de conexões
- [ ] Implementar leitura básica de dados
- [ ] Implementar escrita básica de dados
- [ ] Setup de Makefile com flags obrigatórias

#### Validação com irssi
```bash
irssi
/connect localhost 6667
# Esperado: Conecta (sem resposta do servidor ainda)
```

#### Validação com nc
```bash
nc localhost 6667
# Esperado: Conexão aceita e mantida
```

---

### FASE 2: Autenticação (Semana 1-2)

**Objetivo**: Implementar autenticação com PASS e identidade com NICK/USER.

#### Tarefas
- [ ] Parser IRC básico (separar comandos por `\r\n`)
- [ ] Comando PASS (autenticação)
- [ ] Comando NICK (definir nickname)
- [ ] Comando USER (definir username)
- [ ] Estados de conexão (not-auth → authed → registered)
- [ ] Agregação de pacotes fragmentados

#### Validação com irssi
```bash
# Teste de conexão completa
irssi
/connect localhost 6667
/quote PASS senha123
/nick alice
/quote USER alice 0 * :Alice
# Esperado: Bem-vindo!
```

#### Validação com nc (fragmentado)
```bash
(echo "PASS sen"; sleep 0.5; echo "ha123") | nc localhost 6667
# Esperado: Aguarda segundo pacote, processa quando completo
```

---

### FASE 3: Canais Básicos (Semana 2-3)

**Objetivo**: Implementar JOIN e broadcast de mensagens.

#### Tarefas
- [ ] Estrutura de dados de Canais
- [ ] Comando JOIN
- [ ] Comando PRIVMSG em canais
- [ ] Broadcast para membros
- [ ] Notificações de entrada/saída
- [ ] Comando QUIT

#### Validação com irssi
```bash
# Terminal 1
irssi
/connect localhost 6667
/quote PASS senha123
/nick alice
/join #geral
/msg #geral olá!

# Terminal 2
irssi
/connect localhost 6667
/quote PASS senha123
/nick bob
/join #geral
# Esperado: bob vê alice entrar e a mensagem "olá!"
```

---

### FASE 4: Mensagens Privadas (Semana 3)

**Objetivo**: Implementar PRIVMSG entre usuários.

#### Tarefas
- [ ] PRIVMSG para usuários específicos
- [ ] Roteamento de mensagens privadas
- [ ] Notificação quando usuário não existe

#### Validação com irssi
```bash
# Terminal 1 (alice)
/msg bob mensagem privada

# Terminal 2 (bob)
# Esperado: Recebe mensagem privada de alice
```

---

### FASE 5: Permissões de Canal (Semana 3-4)

**Objetivo**: Distinguir operadores e usuários regulares.

#### Tarefas
- [ ] Estrutura de operadores por canal
- [ ] Definição de operador ao criar canal
- [ ] Validação de permissões antes de executar comandos
- [ ] Rejeitar comandos restritos para usuários regulares

#### Validação com irssi
```bash
# alice é operadora (criou o canal)
# bob é usuário regular
# bob tenta:
/quote KICK #geral alice
# Esperado: Erro "You're not a channel operator"
```

---

### FASE 6: Comando KICK (Semana 4)

**Objetivo**: Implementar remoção de usuários.

#### Tarefas
- [ ] Parser para KICK
- [ ] Validação de permissão
- [ ] Remover usuário do canal
- [ ] Notificar usuário removido
- [ ] Notificar outros membros

#### Validação com irssi
```bash
# alice (operadora)
/quote KICK #geral bob razão

# bob
# Esperado: Sai do #geral com razão
```

---

### FASE 7: Comando INVITE (Semana 4)

**Objetivo**: Implementar convites para canais.

#### Tarefas
- [ ] Comando INVITE
- [ ] Modo de canal +i (invite-only)
- [ ] Validação de invites
- [ ] Notificações de invite

#### Validação com irssi
```bash
# alice (operadora)
/quote MODE #privado +i
/quote INVITE bob #privado

# bob
/join #privado
# Esperado: Consegue entrar apenas com convite
```

---

### FASE 8: Comando TOPIC (Semana 4-5)

**Objetivo**: Implementar tópicos de canal.

#### Tarefas
- [ ] Parser para TOPIC
- [ ] Armazenar tópico por canal
- [ ] Modo +t (restrição de tópico para operadores)
- [ ] Validação de permissão
- [ ] Notificar mudança de tópico

#### Validação com irssi
```bash
# alice (operadora)
/topic #geral Bem-vindo ao canal geral!

# bob
/join #geral
# Esperado: Vê o tópico ao entrar

# bob tenta (se modo +t ativo)
/topic #geral novo tópico
# Esperado: Erro, apenas operadores
```

---

### FASE 9: Comando MODE (Semana 5)

**Objetivo**: Implementar sistema de modos de canal (5 submodos obrigatórios).

#### Tarefas
- [ ] Parser de MODE com submodos
- [ ] Modo `i` (invite-only)
- [ ] Modo `k` (channel key/password)
- [ ] Modo `l` (user limit)
- [ ] Modo `o` (operator privilege)
- [ ] Modo `t` (topic restriction)
- [ ] Armazenar estado de modos
- [ ] Validar operações baseado em modos

#### Validação com irssi
```bash
# alice (operadora)
/quote MODE #dev +i +k senha123 +l 10
# Esperado: Canal fica privado, com senha, e máx 10 usuários

/quote MODE #dev +o bob
# Esperado: bob vira operador
```

---

### FASE 10: Robustez e Edge Cases (Semana 5-6)

**Objetivo**: Garantir estabilidade em situações extremas.

#### Tarefas
- [ ] Testes de flood
- [ ] Testes de desconexão abrupta
- [ ] Testes de comandos fragmentados
- [ ] Verificação de memory leaks
- [ ] Teste com múltiplas conexões (20+)
- [ ] Teste de suspensão (Ctrl-Z)

#### Validação com nc
```bash
# Flood simples
for i in {1..100}; do
  echo "PRIVMSG #test :mensagem $i"
done | nc localhost 6667

# Esperado: Servidor continua respondendo, sem crash
```

---

### FASE 11: Testes Finais (Semana 6)

**Objetivo**: Validação contra todos os requisitos.

#### Tarefas
- [ ] Teste com irssi: autenticação completa
- [ ] Teste com irssi: canais múltiplos
- [ ] Teste com irssi: permissões
- [ ] Teste com irssi: todos os comandos obrigatórios
- [ ] Teste com nc: edge cases
- [ ] Verificação de vazamento de memória
- [ ] Validação contra rubric
- [ ] Documentação final

---

## 📊 Timeline Recomendada

| Semana | Fases | Resultado |
|--------|-------|-----------|
| 1 | 1-2 | Servidor aceita e autentica clientes |
| 2 | 3-4 | Canais e mensagens funcionais |
| 3 | 5-6 | Permissões e KICK implementados |
| 4 | 7-9 | INVITE, TOPIC, MODE completos |
| 5 | 10 | Testes de robustez e edge cases |
| 6 | 11 | Validação final e documentação |

---

## 🎯 Validação por Requisito

### ✅ Requisitos Funcionais Obrigatórios

| Requisito | Fase | Teste com irssi |
|-----------|------|-----------------|
| Autenticação | 2 | `/quote PASS senha` |
| Nickname | 2 | `/nick alice` |
| Username | 2 | `/quote USER alice 0 * :Alice` |
| JOIN | 3 | `/join #canal` |
| PRIVMSG canal | 3 | `/msg #canal olá` |
| PRIVMSG privado | 4 | `/msg usuario olá` |
| Broadcast | 3 | Múltiplos usuários no canal |
| KICK | 6 | `/quote KICK #canal usuario` |
| INVITE | 7 | `/quote INVITE usuario #canal` |
| TOPIC | 8 | `/topic #canal novo tópico` |
| MODE (+i) | 9 | `/quote MODE #canal +i` |
| MODE (+k) | 9 | `/quote MODE #canal +k senha` |
| MODE (+l) | 9 | `/quote MODE #canal +l 5` |
| MODE (+o) | 9 | `/quote MODE #canal +o usuario` |
| MODE (+t) | 9 | `/quote MODE #canal +t` |

### ✅ Requisitos Não-Funcionais

| Requisito | Teste | Validação |
|-----------|-------|-----------|
| Compilação C++98 | `make` | Sem warnings com `-Wall -Wextra -Werror` |
| Um poll() | Análise de código | Grep por `poll()` deve retornar 1 resultado |
| I/O não-bloqueante | Teste de flood | Servidor responde mesmo com 100+ msgs |
| Múltiplas conexões | irssi + nc simultâneos | 10+ conexões sem travar |
| Memory leaks | valgrind | Zero leaks em teste de carga |
| Crash | Stress test | Zero crashes em qualquer cenário |

---

## 🔍 Checklist de Implementação

### Estrutura Base
- [ ] Makefile com regras `$(NAME)`, `all`, `clean`, `fclean`, `re`
- [ ] Compilação com `c++` e flags `-Wall -Wextra -Werror -std=c++98`
- [ ] Executável nomeado `ircserv`
- [ ] Aceita argumentos `./ircserv <port> <password>`

### Multiplexação
- [ ] Um único `poll()` (ou `select()`, `epoll()`, `kqueue()`)
- [ ] `poll()` chamado antes de `accept()`, `read()`, `write()`
- [ ] Nenhum uso de `errno` para disparar ações
- [ ] `fcntl(fd, F_SETFL, O_NONBLOCK)` para cada novo FD
- [ ] Sem forking ou threads

### Protocolo IRC
- [ ] Parser de comandos IRC
- [ ] Agregação de pacotes fragmentados
- [ ] Separação por `\r\n`
- [ ] Estados de cliente (não-autenticado → autenticado → registrado)

### Funcionalidades Core
- [ ] PASS (autenticação)
- [ ] NICK (nickname)
- [ ] USER (username)
- [ ] JOIN (entrar em canal)
- [ ] PART/QUIT (sair de canal/servidor)
- [ ] PRIVMSG (mensagens)

### Comandos de Operador
- [ ] KICK
- [ ] INVITE
- [ ] TOPIC
- [ ] MODE (com 5 submodos)

### Robustez
- [ ] Desconexão abrupta não afeta servidor
- [ ] Comandos fragmentados são processados corretamente
- [ ] Múltiplas conexões não travam servidor
- [ ] Sem memory leaks
- [ ] Zero crashes

---

## 🧪 Testes Recomendados Durante Desenvolvimento

### Teste Diário
```bash
make                          # Compilation check
./ircserv 6667 teste &        # Start server
irssi                         # Quick functional test
ps aux | grep ircserv         # Verify running
kill %1                       # Stop server
```

### Teste Semanal
```bash
# Memory leak check
valgrind --leak-check=full ./ircserv 6667 teste &
# Run through all phases in irssi

# Flood test
for i in {1..100}; do 
  echo "msg"; 
done | nc localhost 6667
```

### Teste Final
```bash
# Full compliance check
./run-all-tests.sh            # Se você criar script
# Validar contra rubric.md
```

---

## 📚 Referência Arquitetural

Consulte:
- [`.github/docs/bircd-reference.md`](.github/docs/bircd-reference.md) - Referência em C
- [`.github/docs/irssi-testing-guide.md`](.github/docs/irssi-testing-guide.md) - Detalhes de cada comando
- [`/docs/architectural-design.md`](/docs/architectural-design.md) - Padrão Reactor

---

## 💡 Dicas de Produtividade

1. **Comece simples**: Faça o servidor aceitar conexões antes de implementar IRC
2. **Teste frequentemente**: Não deixe acumular features sem testar
3. **Use irssi para funcionalidade**: Teste RFC compliance
4. **Use nc para edge cases**: Teste desconexões abruptas, fragments
5. **Automatize testes**: Crie scripts para validar cada fase
6. **Debug com logs**: Imprima eventos importantes para troubleshoot

---

## ✅ Conclusão

Seguindo esta estratégia, você terá:
- ✅ Desenvolvimento incremental e validável
- ✅ Requisitos claros por fase
- ✅ Testes concretos com irssi
- ✅ Documentação de progresso
- ✅ Código pronto para avaliação

**Tempo estimado**: 4-6 semanas para projeto completo e robusto.
