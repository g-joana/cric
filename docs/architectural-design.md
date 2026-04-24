# Arquitetura do Projeto ft_irc

## 🎯 Padrão Arquitetural: Reactor Pattern

O projeto `ft_irc` segue o **Reactor Pattern**, um padrão de design amplamente usado em servidores de rede. É a abordagem correta para implementar um servidor IRC escalável em ambiente single-threaded.

---

## 📚 O que é o Reactor Pattern?

O **Reactor Pattern** é um padrão de design para **multiplexação e dispatching de eventos** em aplicações I/O intensivas.

### Componentes:

1. **Demultiplexer** (`poll()` ou `select()`)
   - Aguarda eventos em múltiplos canais de I/O
   - Retorna quando um ou mais eventos estão prontos

2. **Dispatcher** (event loop)
   - Coordena a execução de handlers apropriados
   - Redireciona eventos para os handlers corretos

3. **Event Handlers** (funções de processamento)
   - Processam os eventos específicos
   - Implementam a lógica de negócio

4. **Connection/I/O Handlers**
   - Gerenciam recursos (sockets, buffers, estado)

---

## 🔄 Fluxo do Reactor Pattern

```
┌─────────────────────────────────────────┐
│         Main Application Loop           │
└────────────┬────────────────────────────┘
             │
             ↓
┌─────────────────────────────────────────┐
│    Demultiplexer (poll/select)          │
│  Aguarda eventos em FDs monitorados     │
└────────────┬────────────────────────────┘
             │
      ┌──────┴──────┐
      ↓             ↓
  [Cliente A]   [Servidor]
  [Cliente B]   [Cliente C]
      │             │
      └──────┬──────┘
             ↓
┌─────────────────────────────────────────┐
│    Dispatcher (Event Loop)              │
│  Identifica quais FDs têm eventos       │
└────────────┬────────────────────────────┘
             │
      ┌──────┼──────┐
      ↓      ↓      ↓
  [Handler] [Handler] [Handler]
   (Read)   (Write)  (Accept)
      │      │      │
      └──────┼──────┘
             ↓
    Processamento da
      Lógica IRC
```

---

## 🏛️ Semelhança com Arquiteturas Reais

### Servidores Que Usam Reactor Pattern

| Servidor | Demultiplexer | Linguagem | Notas |
|----------|---------------|-----------|-------|
| **BIND** (DNS) | `select()` | C | Clássico, legacy |
| **Apache HTTP** (prefork) | `select()` | C | Histórico |
| **Nginx** | `epoll()` / `kqueue()` | C | Moderno, alta performance |
| **Node.js** (libuv) | `epoll()` / `kqueue()` / `ioctl()` | C/JS | Event-driven |
| **Redis** | `epoll()` / `kqueue()` / `select()` | C | In-memory datastore |
| **Memcached** | `select()` / `libevent` | C | Cache distribuído |

### Para `ft_irc`:

O projeto usa a mesma abordagem que **Redis** e **Memcached**:
- Demultiplexer: `poll()` (você pode usar também `select()`, `epoll()`, `kqueue()`)
- Padrão: Reactor Pattern (event-driven, single-threaded)
- Linguagem: C++ (em vez de C)

---

## 🏗️ Estrutura Geral do `ft_irc`

```
┌─────────────────────────────────────────────────┐
│              ft_irc Server                      │
├─────────────────────────────────────────────────┤
│                                                 │
│  ┌────────────────────────────────────────┐    │
│  │     Main Event Loop (main_loop)        │    │
│  │  • Init poll()                         │    │
│  │  • Check events                        │    │
│  │  • Process handlers                    │    │
│  └────────────────────────────────────────┘    │
│                    ↓                            │
│  ┌────────────────────────────────────────┐    │
│  │    Demultiplexer (poll())              │    │
│  │  • Monitora: Servidor + Clientes       │    │
│  │  • Retorna: FDs prontos para I/O       │    │
│  └────────────────────────────────────────┘    │
│                    ↓                            │
│  ┌────────────────────────────────────────┐    │
│  │    Event Handlers                      │    │
│  │  • Accept new connections              │    │
│  │  • Read client data                    │    │
│  │  • Write to clients                    │    │
│  └────────────────────────────────────────┘    │
│                    ↓                            │
│  ┌────────────────────────────────────────┐    │
│  │    IRC Protocol Layer                  │    │
│  │  • Parse commands (PRIVMSG, JOIN, etc) │    │
│  │  • Aggregate fragmented packets        │    │
│  │  • Route messages                      │    │
│  └────────────────────────────────────────┘    │
│                    ↓                            │
│  ┌────────────────────────────────────────┐    │
│  │    Business Logic                      │    │
│  │  • Channel management                  │    │
│  │  • User permissions                    │    │
│  │  • Mode handlers (KICK, INVITE, etc)   │    │
│  │  • State management                    │    │
│  └────────────────────────────────────────┘    │
│                    ↓                            │
│  ┌────────────────────────────────────────┐    │
│  │    Data Persistence Layer              │    │
│  │  • Channels                            │    │
│  │  • Users                               │    │
│  │  • Buffers                             │    │
│  └────────────────────────────────────────┘    │
│                                                 │
└─────────────────────────────────────────────────┘
```

---

## 🔑 Componentes Principais

### 1. **Event Loop** (Reactor)
```cpp
void main_loop(Server &server) {
  while (true) {
    poll(server.fds, server.nfds, timeout);
    dispatch_events(server);
  }
}
```

### 2. **Connection Handlers**
```cpp
// Quando um cliente se conecta
handle_accept(int server_fd);

// Quando há dados a ler
handle_read(int client_fd);

// Quando é possível escrever
handle_write(int client_fd);
```

### 3. **Protocol Layer** (IRC Parser)
```cpp
class IRCCommand {
  std::string command;
  std::vector<std::string> params;
  std::string execute(Server &server);
};
```

### 4. **Business Logic** (Canais, Usuários, Modos)
```cpp
class Channel {
  std::string name;
  std::vector<User> members;
  void broadcast_message(std::string msg);
  void kick_user(std::string user);
};
```

---

## ⚙️ Fluxo de Funcionamento Detalhado

### Cenário: Cliente envia `PRIVMSG #canal :oi`

```
1. poll() retorna → Socket cliente pronto para leitura
   ↓
2. handle_read(client_fd) é chamado
   ↓
3. recv() lê dados do socket → armazena em buffer
   ↓
4. Agregar pacotes fragmentados
   ↓
5. Dividir em comandos individuais (separados por \r\n)
   ↓
6. Para cada comando:
   ├─ Parse: "PRIVMSG #canal :oi"
   ├─ Cria objeto IRCCommand
   ├─ Executa lógica:
   │  ├─ Verifica se cliente tem permissão
   │  ├─ Valida parâmetros
   │  ├─ Recupera canal #canal
   │  ├─ Enfileira mensagem para broadcast
   │  └─ Enfileira respostas ao cliente
   │
7. poll() retorna → Sockets clientes prontos para escrita
   ↓
8. handle_write() envia respostas enfileiradas via send()
   ↓
9. Volta ao loop...
```

---

## 📊 Vantagens do Reactor Pattern para IRC

| Vantagem | Benefício |
|----------|-----------|
| **Single-threaded** | Sem race conditions, sem locks |
| **Escalável** | Milhares de conexões simultâneas |
| **Responsivo** | Event-driven, nunca bloqueia |
| **Simples** | Lógica sequencial, fácil de debugar |
| **Requisito** | Conforme mandatório do projeto |

---

## ⚠️ Desafios e Soluções

### Desafio 1: Pacotes Fragmentados

**Problema**: TCP pode dividir um comando em múltiplos pacotes

```
Pacote 1: "PRIVMSG #canal"
Pacote 2: " :olá\r\n"
```

**Solução**: Manter buffer de entrada por cliente, agregar até encontrar `\r\n`

### Desafio 2: Cliente Não Lê Respostas Rápido

**Problema**: Socket cliente preenchido, `send()` retorna -1 (EAGAIN)

**Solução**: Enfileirar resposta, tentar novamente quando `poll()` indicar escrita possível

### Desafio 3: Cliente Cai Abruptamente

**Problema**: `recv()` retorna 0 (conexão fechada)

**Solução**: Detectar e limpar estruturas, informar outros clientes

### Desafio 4: Múltiplos Comandos Num Pacote

**Problema**: `"PRIVMSG a :x\r\nPRIVMSG b :y\r\n"` vem de uma vez

**Solução**: Loop parsing enquanto houver `\r\n` no buffer

---

## 🔬 Diagrama: Estados de um Cliente

```
             ┌──────────────┐
             │   DISCONNECTED │
             └────────┬──────┘
                      │ accept()
                      ↓
             ┌──────────────────┐
             │  CONNECTED       │
             │  (não autenticado)│
             └────────┬─────────┘
                      │ PASS <password>
         ┌────────────┴───────────┐
         │ Correto               │ Incorreto
         ↓                       ↓
    ┌─────────────┐      ┌──────────────┐
    │  AUTHENTICATED │      │   QUIT      │
    │  NICK/USER?  │      │  (disconnect)│
    └────────┬────┘      └──────────────┘
             │
             │ NICK + USER OK
             ↓
    ┌────────────────┐
    │   REGISTERED   │
    │   Pronto para  │
    │   comandos     │
    └────────┬───────┘
             │
      ┌──────┼──────┬──────┐
      ↓      ↓      ↓      ↓
    JOIN  PRIVMSG PART  MODE (etc)
      │      │      │      │
      └──────┴──────┴──────┘
             │
             ↓ (ao sair de todos os canais)
    ┌────────────────┐
    │   DISCONNECTED │
    │  (via QUIT)    │
    └────────────────┘
```

---

## 📝 Resumo da Arquitetura

1. **Padrão**: Reactor Pattern (event-driven, single-threaded)
2. **Demultiplexer**: `poll()` (conforme mandatório)
3. **Loop Principal**: Aguarda → Processa → Repete
4. **Handlers**: Accept, Read, Write (não-bloqueantes)
5. **Protocolo**: IRC (RFC 1459/2812)
6. **Estado**: Máquina de estados por cliente
7. **Segurança**: Agregação de pacotes, validação de comandos
8. **Performance**: Zero threads, zero locks

---

## �️ Próximos Passos com Guia de Desenvolvimento

Para uma abordagem **iterativa e estruturada** do desenvolvimento, consulte:

### 📋 [`.github/docs/development-strategy.md`](.github/docs/development-strategy.md)
- **11 Fases de desenvolvimento** com tarefas específicas
- **Timeline recomendada**: 4-6 semanas
- **Validação por requisito** funcional
- **Testes estruturados** por fase
- **Roadmap claro** do conceitual ao pronto para produção

### 🧪 [`.github/docs/irssi-testing-guide.md`](.github/docs/irssi-testing-guide.md)
- **Cliente de referência**: irssi (RFC 1459/2812 compliant)
- **Testes concretos** para cada comando
- **Exemplos de uso** do irssi
- **Scripts de teste** automatizados
- **Troubleshooting** e dicas

### 📖 [`.github/docs/bircd-reference.md`](.github/docs/bircd-reference.md)
- **Arquitetura de referência** em C
- **Estruturas de dados** principais
- **Fluxo de execução** detalhado
- **Mapeamento** de conceitos ao seu projeto

---

## 📝 Resumo da Arquitetura

1. **Padrão**: Reactor Pattern (event-driven, single-threaded)
2. **Demultiplexer**: `poll()` (conforme mandatório)
3. **Loop Principal**: Aguarda → Processa → Repete
4. **Handlers**: Accept, Read, Write (não-bloqueantes)
5. **Protocolo**: IRC (RFC 1459/2812)
6. **Estado**: Máquina de estados por cliente
7. **Segurança**: Agregação de pacotes, validação de comandos
8. **Performance**: Zero threads, zero locks

---

## 📚 Próximos Passos

1. Estudar `bircd/` em `.github/docs/bircd-reference.md`
2. Seguir **estratégia de desenvolvimento** em `.github/docs/development-strategy.md`
3. Usar **irssi** para testes segundo `.github/docs/irssi-testing-guide.md`
4. Implementar Event Loop básico com `poll()`
5. Adicionar aceitação de conexões
6. Implementar buffers (read + write)
7. Implementar parser IRC
8. Adicionar handlers de comandos
9. Testar com `nc` e cliente IRC real

---

## 🔗 Referências

- [Reactor Pattern - Explanation](https://en.wikipedia.org/wiki/Reactor_pattern)
- [Scalable I/O with Reactive Streams](https://www.oilshell.org/blog/2017/08/20.html)
- [Redis Architecture](https://redis.io/topics/protocol-spec)
- [RFC 1459 - IRC Protocol](https://tools.ietf.org/html/rfc1459)
- Pattern: *Patterns of Distributed Systems* by Sam Newman
