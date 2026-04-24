# ft_irc - Requisitos Completos para Desenvolvimento

## 1. Regras Gerais de Desenvolvimento

### Estabilidade
* O programa não deve crashar em nenhuma circunstância (mesmo sem memória) e não deve encerrar inesperadamente.
* Caso ocorra um crash, o projeto será considerado não funcional (nota 0).
* Não são permitidos *segmentation faults* ou encerramentos inesperados do programa.

### Compilação
* Deve ser entregue um **Makefile** que compile os arquivos fonte.
* Regras obrigatórias: `$(NAME)`, `all`, `clean`, `fclean` e `re`.
* Compilador: `c++`.
* Flags exigidas: `-Wall -Wextra -Werror`.
* Deve compilar com a flag `-std=c++98`.

### Padrão de Código
* O código deve estar em conformidade com o padrão **C++98**.
* Priorizar recursos de C++ em vez de C (ex: `<cstring>` em vez de `<string.h>`).

### Bibliotecas
* O uso de bibliotecas externas e da biblioteca Boost é proibido.
* A `Libft` não é autorizada para este projeto.

### Gerenciamento de Memória
* Toda memória alocada na heap deve ser devidamente liberada antes do fim da execução.
* O servidor deve ser verificado quanto a vazamentos de memória (leaks) durante operações de carga/flood.

---

## 2. Especificações Técnicas (Mandatório)

### Executável
* O nome do programa deve ser `ircserv`.
* O programa deve aceitar dois argumentos na execução: `./ircserv <port> <password>`.
  * `port`: A porta na qual o servidor ouvirá conexões (deve escutar em todas as interfaces de rede).
  * `password`: Senha necessária para a conexão de qualquer cliente IRC.

### Arquitetura de Rede

#### Protocolo e Comunicação
* A comunicação deve ser via **TCP/IP (v4 ou v6)**.
* O servidor deve iniciar e escutar em todas as interfaces de rede na porta fornecida.

#### Multiplexação de I/O (Crítico)
* **Proibição de Forking**: Todo I/O deve ser **não-bloqueante**.
* Apenas **1 poll()** (ou equivalente como `select()`, `kqueue()` ou `epoll()`) deve ser usado para gerenciar **todas** as operações (leitura, escrita, escuta, etc.).
* O `poll()` (ou equivalente) deve ser chamado **antes** de cada operação de `accept`, `read/recv` e `write/send`.
* **Proibição de errno**: O `errno` não deve ser usado para disparar ações específicas após essas chamadas (ex: tentar ler novamente após um `EAGAIN`).
* Tentar ler/receber ou escrever/enviar em qualquer file descriptor sem usar o `poll()` (ou equivalente) resulta em nota 0.

#### Uso de fcntl
* Qualquer chamada para `fcntl()` deve ser feita estritamente no formato: `fcntl(fd, F_SETFL, O_NONBLOCK);`.
* Outros usos de `fcntl()` são proibidos.

#### Manipulação de Dados
* Como os pacotes podem chegar fragmentados, é **obrigatório** agregar os pacotes recebidos para reconstruir o comando completo antes de processá-lo.
* O servidor deve lidar corretamente com comandos enviados de forma parcial (reconstruindo o comando) sem afetar outras conexões.

### Conectividade e Clientes
* O servidor deve lidar com múltiplos clientes simultaneamente **sem travar**, respondendo a todas as demandas simultaneamente.
* Deve ser possível conectar ao servidor usando a ferramenta `nc` (netcat), enviar comandos e receber respostas.
* O servidor deve ser testado com múltiplas conexões ao mesmo tempo (cliente IRC + `nc` simultaneamente).

### Comportamento em Situações Extremas

#### Quedas Inesperadas
* Se um cliente for encerrado abruptamente (kill), o servidor deve continuar operacional para os outros clientes e aceitar novas conexões.
* Se o `nc` for encerrado com apenas metade de um comando enviado, o servidor não deve ficar bloqueado ou em estado inconsistente.

#### Flood e Suspensão
* Ao suspender um cliente (`Ctrl-Z`) em um canal e inundar (flood) o canal com outro cliente, o servidor não deve travar.
* Quando o cliente suspenso retornar, os comandos acumulados devem ser processados normalmente.

---

## 3. Requisitos Funcionais (Protocolo IRC)

### Cliente de Referência
* Você deve escolher um cliente IRC real como referência para os testes.
* O servidor deve ser compatível com este cliente sem erros.
* O grupo deve informar qual cliente IRC será utilizado como referência.

### Funcionalidades Básicas
* **Autenticação de usuário**: Autenticar com a senha fornecida na inicialização do servidor.
* **Definição de identidade**: Permitir definir *nickname* e *username*.
* **Entrada em canais**: Comando `JOIN` para entrar em canais.
* **Mensagens privadas**: Suportar `PRIVMSG` com diferentes parâmetros, totalmente funcional.
* **Encaminhamento de mensagens**: Toda mensagem enviada a um canal deve ser transmitida para todos os outros membros desse canal.

### Gestão de Permissões
* Deve haver distinção entre usuários regulares e operadores de canal.
* Usuários regulares não devem ter privilégios para realizar ações de operador.

---

## 4. Comandos de Operador (Obrigatórios)

Os seguintes comandos devem ser implementados especificamente para operadores de canal e devem ser testados e funcionar corretamente:

* **KICK**: Expulsar um cliente do canal.
* **INVITE**: Convidar um cliente para o canal.
* **TOPIC**: Alterar ou visualizar o tópico do canal.
* **MODE**: Alterar as configurações do canal:
  * `i`: Definir/remover canal apenas para convidados (*Invite-only*).
  * `t`: Definir/remover restrição do comando `TOPIC` apenas para operadores.
  * `k`: Definir/remover senha do canal (*key*).
  * `o`: Dar/retirar privilégio de operador de canal.
  * `l`: Definir/remover limite de usuários no canal.

---

## 5. Submissão e Avaliação

### Arquivos a Entregar
* `Makefile` com as regras obrigatórias.
* Arquivos de código-fonte: `*.h`, `*.hpp`, `*.cpp`, `*.tpp`, `*.ipp`.
* Arquivo de configuração (opcional).

### Processo de Defesa
* Durante a avaliação, pode ser solicitada uma modificação breve no projeto (mudar um comportamento, reescrever algumas linhas ou adicionar uma feature simples) para verificar o entendimento real do código.
* A modificação deve ser factível em poucos minutos no seu setup usual.

---

## 6. Parte Bônus

**Observação**: A parte de bônus (transferência de arquivos e bot) só será avaliada se a parte mandatória estiver perfeita.

* **Transferência de Arquivos**: Deve funcionar com o cliente IRC de referência.
* **Bot IRC**: Deve existir um bot IRC funcional.

---

## Notas Críticas para Avaliação

Se qualquer um dos seguintes pontos **básicos** falhar, a avaliação é encerrada com nota 0:

1. ✓ Estrutura e Compilação correta (Makefile, flags, etc.)
2. ✓ Apenas **UM** `poll()` ou equivalente em todo o código
3. ✓ O `poll()` chamado antes de cada `accept`, `read/recv`, `write/send`
4. ✓ Proibição de usar `errno` para disparar ações após `poll()`
5. ✓ Uso correto de `fcntl(fd, F_SETFL, O_NONBLOCK);`
6. ✓ Nenhum segmentation fault ou crash inesperado
