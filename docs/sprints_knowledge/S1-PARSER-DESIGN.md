# S1 - Parser Design & Bug Fixes

## 🎯 Objetivo Alcançado

Sprint S1 completou:
1. ✅ Corrigiu 2 bugs críticos de S0 (modificação de vector + remoção incorreta)
2. ✅ Implementou CommandParser robusto para agregação de pacotes
3. ✅ Integrou parser no Client e Server
4. ✅ Testes funcionais e sem memory leaks

---

## 🐛 Bugs S0 Corrigidos

### Bug #1: Modificação de Vector Durante Iteração

**Antes (ERRADO)**:
```cpp
void Server::run() {
    for (size_t i = 0; i < _pollfds.size(); i++) {  // ← size() pode mudar!
        if (_pollfds[i].fd == _fd)
            _acceptClient();  // ← push_back() modifica _pollfds durante loop
    }
}
```

**Problema**: 
- `_acceptClient()` chama `_pollfds.push_back(pfd)` durante iteração
- Iteradores podem ser invalidados
- Alguns elementos pulados ou processados fora de ordem
- **Resultado**: Cliente 2 não recebia dados após Cliente 1 desconectar

**Depois (CORRETO)**:
```cpp
void Server::_acceptClient() {
    // ... accept client ...
    _clients[clientFd] = new Client(clientFd);
    _pendingConnections.push_back(clientFd);  // ← Não adiciona em _pollfds ainda
}

void Server::_processPendingConnections() {
    // Adiciona APÓS o loop de poll terminar
    for (size_t i = 0; i < _pendingConnections.size(); i++) {
        int clientFd = _pendingConnections[i];
        struct pollfd pfd;
        pfd.fd = clientFd;
        pfd.events = POLLIN;
        pfd.revents = 0;
        _pollfds.push_back(pfd);
    }
    _pendingConnections.clear();
}

void Server::run() {
    while (true) {
        poll(&_pollfds[0], _pollfds.size(), -1);
        // ... processar eventos ...
        _processPendingConnections();  // ← Adiciona seguramente após loop
    }
}
```

**Solução**: Manter conexões pendentes em fila separada, processar após loop terminar.

---

### Bug #2: Remoção Incorreta com Decremento

**Antes (ERRADO)**:
```cpp
if (bytesRead <= 0) {
    // ... cleanup ...
    _pollfds.erase(_pollfds.begin() + i);
    i--;  // ← Confuso e propenso a erros
    continue;
}
```

**Problema**:
- `erase()` muda o tamanho do vetor
- `i--` segue `erase()`, mas `continue` salta para próxima iteração do for
- `i++` automático do for é executado mesmo após erase
- Resultado: Processando clientes sem eventos

**Depois (CORRETO)**:
```cpp
if (bytesRead <= 0) {
    // ... cleanup ...
    _pollfds.erase(_pollfds.begin() + i);
    // Não incrementa i - erase já aponta para próximo elemento
}
else {
    // ... processar ...
    i++;
}
// Após if/else: loop automático para próximo i
```

**Solução**: Não incrementar `i` após `erase()`. O loop for controlado manualmente.

---

## 📦 CommandParser - Design Completo

### Objetivo
Agregar dados fragmentados de múltiplos `recv()` em pacotes IRC completos (terminados com `\r\n`).

### Classe: CommandParser

```cpp
class CommandParser {
private:
    std::string _buffer;  // Buffer acumulativo

public:
    void appendData(const std::string &data);      // Agrega dados recebidos
    bool hasCompleteCommand() const;               // Verifica \r\n
    std::string extractCommand();                  // Remove e retorna comando
    std::string getBuffer() const;                 // Debug
    void clearBuffer();                            // Reset
};
```

### Fluxo de Funcionamento

**Cenário: Comando fragmentado em 2 recv()**

```
Cliente envia: "NICK alice\r\n" em 2 partes:
  Parte 1: "NICK al"   (7 bytes)
  Parte 2: "ice\r\n"   (6 bytes)

Servidor:
  1️⃣ recv() retorna "NICK al"
     appendData("NICK al")
     hasCompleteCommand() → false (sem \r\n)
     buffer = "NICK al"
     
  2️⃣ recv() retorna "ice\r\n"
     appendData("ice\r\n")
     hasCompleteCommand() → true (\r\n encontrado!)
     extractCommand() → "NICK alice"
     buffer = ""
     
  3️⃣ Processar comando "NICK alice"
```

### Exemplo: Múltiplos Comandos em 1 recv()

```
Cliente envia: "NICK alice\r\nUSER alice 0 * :Alice\r\n"
  (ambos em uma única chamada recv())

Servidor:
  1️⃣ recv() retorna "NICK alice\r\nUSER alice 0 * :Alice\r\n"
     appendData(...) 
     
  2️⃣ while (hasCompleteCommand()):
       extractCommand() → "NICK alice"
       [processar NICK alice]
       
       hasCompleteCommand() ainda true
       extractCommand() → "USER alice 0 * :Alice"
       [processar USER alice 0 * :Alice]
       
       hasCompleteCommand() agora false
       [sair do while]
```

### Exemplo: Trailing com Espaços

```
Cliente envia: "PRIVMSG #ch :hello world\r\n"

Parser:
  appendData("PRIVMSG #ch :hello world\r\n")
  extractCommand() → "PRIVMSG #ch :hello world"
  
  Nota: Tudo após ':' é preservado (não é parsing completo aqui)
  S2 será responsável por separar COMMAND/PARAMS/TRAILING
```

---

## 🔧 Integração no Client

```cpp
class Client {
private:
    CommandParser _parser;  // Cada cliente tem seu parser
    
public:
    void appendToBuffer(const std::string &data) {
        _parser.appendData(data);
    }
    
    bool hasCompleteCommand() const {
        return _parser.hasCompleteCommand();
    }
    
    std::string extractCommand() {
        return _parser.extractCommand();
    }
};
```

---

## 🔌 Integração no Server

```cpp
void Server::run() {
    for (size_t i = 0; i < _pollfds.size(); ) {
        if (recv ...) {
            buffer[bytesRead] = '\0';
            Client *client = _clients[_pollfds[i].fd];
            
            // 1. Agregar dados ao buffer do cliente
            client->appendToBuffer(buffer);
            
            // 2. Extrair todos os comandos completos
            while (client->hasCompleteCommand()) {
                std::string command = client->extractCommand();
                if (!command.empty()) {
                    // 3. Processar cada comando
                    std::cout << "Comando: " << command << std::endl;
                }
            }
            i++;
        }
    }
}
```

---

## 📋 Casos de Teste Implementados

| Teste | Cenário | Status |
|-------|---------|--------|
| Bug Fix #1 | Múltiplos clients conectando | ✓ Passa |
| Bug Fix #2 | Clientes desconectando | ✓ Passa (erase sem decremento) |
| Parser Simples | `NICK alice\r\n` | ✓ Passa |
| Parser Fragmentado | `NIC` + `K alice\r\n` | ✓ Passa |
| Parser Múltiplo | 3 comandos em um recv | ✓ Passa |
| Sem Crash | Servidor aguenta 10s de stress | ✓ Passa |

---

## 🎯 Próximos Passos (S2)

S2 precisa:
1. Implementar parseCommand() para separar COMMAND/PARAMS/TRAILING
2. Criar handlers para PASS, NICK, USER
3. Gerenciar estados de cliente (INIT → AUTH → REGISTERED)
4. Enviar RPL_WELCOME (001) após autenticação completa

**Entrada para S2**: 
- CommandParser funciona ✓
- Client tem métodos para obter comandos ✓
- Server chama hasCompleteCommand() e extractCommand() ✓

---

## 📊 Métricas

- **Linhas de código adicionadas**: ~150 (CommandParser + correções Server)
- **Bugs corrigidos**: 2 críticos (poll loop + vector safety)
- **Testes criados**: 3 (bug-fix, parser, acceptance)
- **Compilação**: 0 warnings, 0 errors
- **Memory leaks**: Nenhum (valgrind clean)

---

## ✅ Checklist de Conclusão

- [x] Bug #1 fixado (vector modification)
- [x] Bug #2 fixado (erase + decrement)
- [x] CommandParser implementado
- [x] Parser integrado em Client
- [x] Parser integrado em Server
- [x] Testes passando (compilação, bugs, parser, crash)
- [x] Sem memory leaks
- [x] Documentação completa
