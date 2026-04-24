# Bircd - Referência Arquitetural do Projeto

## 📋 Visão Geral

O diretório `bircd/` contém um servidor IRC básico e educacional escrito em **C com `select()`**, fornecido como **referência arquitetural** para o desenvolvimento do projeto `ft_irc`.

**Importante**: `bircd/` é **apenas uma referência de estrutura**, não código a ser executado ou integrado. Deve ser usado como inspiração para entender como estruturar um servidor IRC multiplexado.

---

## 🏗️ Estrutura do `bircd/`

### Arquivos principais:

| Arquivo | Propósito |
|---------|-----------|
| `bircd.h` | Header principal com estruturas e declarações |
| `main.c` | Ponto de entrada do programa |
| `main_loop.c` | Loop principal do servidor |
| `init_env.c` | Inicialização do ambiente e alocação de recursos |
| `init_fd.c` | Inicialização dos file descriptors |
| `do_select.c` | Chamada ao `select()` |
| `check_fd.c` | Verificação de quais FDs estão prontos |
| `srv_create.c` | Criação do socket servidor |
| `srv_accept.c` | Aceitação de novas conexões |
| `client_read.c` | Leitura de dados do cliente |
| `client_write.c` | Escrita de dados para o cliente |
| `clean_fd.c` | Limpeza/inicialização de estruturas FD |
| `get_opt.c` | Parsing de argumentos da linha de comando |
| `x.c` | Funções utilitárias de erro |

---

## 🔄 Fluxo de Execução

```
main()
  ↓
init_env()          → Aloca array de FDs baseado em limite do sistema
  ↓
get_opt()           → Parse de argumentos (porta)
  ↓
srv_create()        → Cria socket servidor
  ↓
main_loop()         → Loop infinito:
  ├─ init_fd()      → Inicializa fd_sets para select()
  ├─ do_select()    → Aguarda eventos (bloqueante)
  └─ check_fd()     → Processa FDs prontos (read/write/accept)
```

---

## 📊 Estruturas de Dados

### `t_fd` - Representação de um File Descriptor

```c
typedef struct s_fd {
  int    type;              // FD_FREE, FD_SERV ou FD_CLIENT
  void   (*fct_read)();     // Ponteiro para função de leitura
  void   (*fct_write)();    // Ponteiro para função de escrita
  char   buf_read[4097];    // Buffer de entrada
  char   buf_write[4097];   // Buffer de saída
} t_fd;
```

**Tipos de FD**:
- `FD_FREE` (0): Não está em uso
- `FD_SERV` (1): Socket servidor (aceita conexões)
- `FD_CLIENT` (2): Socket cliente (troca dados)

### `t_env` - Ambiente Global

```c
typedef struct s_env {
  t_fd      *fds;        // Array dinâmico de todos os FDs
  int       port;        // Porta do servidor
  int       maxfd;       // Máximo de FDs (limite do SO)
  int       max;         // Maior FD atualmente em uso
  int       r;           // Resultado do select()
  fd_set    fd_read;     // Set para select() - leitura
  fd_set    fd_write;    // Set para select() - escrita
} t_env;
```

---

## 🔑 Conceitos Chave

### 1. **Multiplexação com `select()`**

O `select()` monitora múltiplos file descriptors simultaneamente:

```c
int select(int nfds, fd_set *readfds, fd_set *writefds,
           fd_set *exceptfds, struct timeval *timeout);
```

No `bircd`:
- **readfds**: Monitora quais FDs têm dados a ler
- **writefds**: Monitora quais FDs estão prontos para escrita
- **nfds**: `max + 1` (maior FD + 1)
- **timeout**: `NULL` (bloqueante - aguarda indefinidamente)

### 2. **I/O Não-Bloqueante**

Embora não apareça explicitamente no `bircd`, a ideia é que:
- Cada FD está configurado como **não-bloqueante** (com `fcntl(fd, F_SETFL, O_NONBLOCK)`)
- O `select()` diz quando é **seguro** ler/escrever sem travar

### 3. **Padrão Reactor**

O `bircd` implementa um **Reactor Pattern**:
- Um único event loop aguarda eventos
- Quando eventos ocorrem, callbacks são executados
- Cada FD tem ponteiros (`fct_read`, `fct_write`) para funções específicas

---

## 🔗 Mapeamento para `ft_irc`

### O que Manter/Adaptar:

| Conceito `bircd/` | Adaptação para `ft_irc` |
|-------------------|------------------------|
| Array de `t_fd` | Manter conceito, mas com classes C++ |
| `select()` | Usar `poll()` conforme requisitos |
| Loop principal | Mesmo padrão: init → aguarda → processa |
| Buffers read/write | Agregar dados fragmentados |
| Dispatch callbacks | Implementar parser IRC e comandos |

### O que Adicionar para `ft_irc`:

- ✅ Protocolo IRC completo (parsing de comandos)
- ✅ Implementação de canais e gerenciamento de usuários
- ✅ Autenticação com senha
- ✅ Comandos obrigatórios: KICK, INVITE, TOPIC, MODE
- ✅ Tratamento de estado complexo (permissões, modos)
- ✅ Compatibilidade com clientes IRC reais

---

## ⚠️ Diferenças Críticas: `bircd/` vs `ft_irc`

| Aspecto | `bircd/` | `ft_irc` |
|---------|----------|---------|
| **Linguagem** | C (K&R style) | **C++98** |
| **Multiplexação** | `select()` | `poll()` **(mandatório)** |
| **Protocolo** | Nenhum (genérico) | **IRC completo** |
| **Canais** | ❌ Não tem | ✅ Obrigatório |
| **Comandos** | ❌ Nenhum | ✅ 7+ comandos |
| **Autenticação** | ❌ Nenhuma | ✅ Com senha |
| **Compilação** | `gcc` | `c++ -Wall -Wextra -Werror -std=c++98` |
| **Estabilidade** | Básica | **Zero crashes em qualquer situação** |

---

## 💡 Como Usar Esta Referência

1. **Estude a estrutura geral**: Como dados fluem pelo servidor
2. **Entenda o event loop**: init → select → process
3. **Observe os buffers**: Como dados são armazenados/recuperados
4. **Reimplemente em C++**: Não copie o código, adapte os conceitos
5. **Adicione a lógica IRC**: Parsing, comandos, estado
6. **Teste com `poll()`**: Conforme requisitos obrigatórios

---

## 🧪 Cliente de Referência: irssi

Para testes e validação de conformidade com este projeto, use **irssi**:

- **Instalação**: `sudo apt install irssi`
- **Compatibilidade**: 100% RFC 1459/2812
- **Uso**: Consulte [`.github/docs/irssi-testing-guide.md`](.github/docs/irssi-testing-guide.md)

irssi permite validar cada requisito funcional de forma interativa e é o padrão da indústria.

---

## 📚 Referências Recomendadas

- [RFC 1459 - Internet Relay Chat Protocol](https://tools.ietf.org/html/rfc1459)
- [RFC 2812 - Internet Relay Chat: Client Protocol](https://tools.ietf.org/html/rfc2812)
- `man select` - Documentação do select()
- `man poll` - Documentação do poll() (para `ft_irc`)
- Architectural Pattern: Reactor Pattern (implementado aqui)
- [irssi Documentation](https://irssi.org/)

---

## 📝 Notas para Próximas Iterações

- Este é um **servidor muito básico** - não faz nada de IRC específico
- Priorize **implementar o protocolo IRC** antes de otimizações
- Use `poll()` **conforme mandatório** no projeto
- Teste frequentemente com `nc` e um cliente IRC real
- Monitore **memory leaks** durante testes de carga (flood)
