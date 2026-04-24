# irssi - Cliente de Referência para ft_irc

## 📋 Visão Geral

**irssi** é o cliente IRC de referência escolhido para testes e validação do projeto `ft_irc`. É um cliente lightweight, totalmente compatível com RFC 1459/2812, amplamente usado em produção e extremamente adequado para testes de conformidade.

---

## 🛠️ Instalação

### Linux (Debian/Ubuntu)
```bash
sudo apt update
sudo apt install irssi
```

### Linux (Fedora/RHEL)
```bash
sudo dnf install irssi
```

### macOS
```bash
brew install irssi
```

### Verificar Instalação
```bash
irssi --version
```

---

## 🚀 Primeiras Conexões

### Iniciar irssi
```bash
irssi
```

Você verá a interface do irssi com um prompt no topo.

### Conectar ao Servidor ft_irc

Dentro do irssi:
```
/connect localhost 6667
```

Resposta esperada:
```
* Looking up localhost...
* Connecting to localhost [127.0.0.1] port 6667...
```

### Autenticar com Senha

Após conectar:
```
/quote PASS sua_senha_aqui
```

**Nota**: O comando `/quote` envia comandos IRC brutos sem processamento.

### Definir Nickname e Username

```
/nick seu_apelido
/quote USER seu_user 0 * :Real Name
```

Resposta esperada (se bem-sucedido):
```
* Welcome seu_apelido!
```

### Verificar Status

Para verificar se está autenticado:
```
/quote WHOIS seu_apelido
```

---

## 📚 Guia de Testes por Fase

### ✅ FASE 1: Conectividade Básica

**Objetivo**: Validar que o servidor aceita conexões.

#### Teste 1.1 - Conexão TCP
```bash
# Terminal 1: Iniciar seu servidor
./ircserv 6667 senha123

# Terminal 2: Testar com nc
nc localhost 6667
# Esperado: Conexão é aceita
# Digite: hello
# Esperado: Desconecta silenciosamente (servidor não reconhece comando)
```

#### Teste 1.2 - Múltiplas Conexões
```bash
# Terminal 1: Servidor
./ircserv 6667 senha123

# Terminal 2, 3, 4: Cada um executa
nc localhost 6667
# Esperado: Todos conseguem conectar simultaneamente
```

---

### ✅ FASE 2: Autenticação

**Objetivo**: Validar que servidor rejeita clientes sem senha correta.

#### Teste 2.1 - Senha Correta
```bash
# irssi
/connect localhost 6667
/quote PASS senha123
/nick test
/quote USER test 0 * :Test User
# Esperado: Bem-vindo! Cliente autenticado.
```

#### Teste 2.2 - Senha Incorreta
```bash
# irssi
/connect localhost 6667
/quote PASS senhaerrada
/nick test
/quote USER test 0 * :Test User
# Esperado: Erro de autenticação, desconexão
```

#### Teste 2.3 - Sem Senha
```bash
# nc
echo -e "NICK test\r\nUSER test 0 * :Test\r\n" | nc localhost 6667
# Esperado: Desconexão ou erro
```

---

### ✅ FASE 3: Identidade (NICK + USER)

**Objetivo**: Validar que cliente pode definir nickname e username.

#### Teste 3.1 - Mudar Nickname
```bash
# irssi
/connect localhost 6667
/quote PASS senha123
/nick usuario1
/quote USER user1 0 * :User One
# Em outro irssi, faça /users
# Esperado: usuario1 aparece na lista
```

#### Teste 3.2 - Nickname Duplicado
```bash
# irssi 1
/connect localhost 6667
/quote PASS senha123
/nick alice

# irssi 2
/connect localhost 6667
/quote PASS senha123
/nick alice
# Esperado: Erro, não consegue usar nickname repetido
```

---

### ✅ FASE 4: Canais (JOIN)

**Objetivo**: Validar entrada em canais e visibilidade.

#### Teste 4.1 - Entrar em Canal
```bash
# irssi 1
/connect localhost 6667
/quote PASS senha123
/nick alice
/join #geral
# Esperado: Bem-vindo ao #geral

# irssi 2
/connect localhost 6667
/quote PASS senha123
/nick bob
/join #geral
# Esperado: Bem-vindo ao #geral, vê alice
# Esperado: alice recebe notificação que bob entrou
```

#### Teste 4.2 - Múltiplos Canais
```bash
# irssi 1
/join #geral
/join #dev
/join #random
/quote CHANNELS
# Esperado: Vê os 3 canais
```

#### Teste 4.3 - Sair de Canal
```bash
# irssi 1
/join #geral
/part #geral
# Ou simplesmente
/quit
# Esperado: bob recebe notificação que alice saiu
```

---

### ✅ FASE 5: Mensagens Privadas (PRIVMSG)

**Objetivo**: Validar envio de mensagens entre clientes.

#### Teste 5.1 - Mensagem em Canal
```bash
# irssi 1
/join #geral
/msg #geral olá pessoal!

# irssi 2
/join #geral
# Esperado: Vê "alice: olá pessoal!" no #geral
```

#### Teste 5.2 - Mensagem Privada
```bash
# irssi 1
/msg bob mensagem privada

# irssi 2
# Esperado: Recebe msg privada de alice
```

#### Teste 5.3 - Mensagens com Parâmetros
```bash
# irssi 1
/msg alice :mensagem com espaços
/msg #geral :outra mensagem
```

---

### ✅ FASE 6: Permissões e Operadores

**Objetivo**: Validar que apenas operadores podem executar comandos.

#### Teste 6.1 - Dar Operador
```bash
# irssi 1 (criador do canal)
/join #dev
/quote MODE #dev +o alice
# alice agora é operadora do #dev
```

#### Teste 6.2 - Usuário Regular Não Pode Kickar
```bash
# irssi 2 (sem privilégios)
/join #dev
/quote KICK #dev alice
# Esperado: Erro, permissão negada
```

#### Teste 6.3 - Operador Pode Kickar
```bash
# irssi 1 (operadora)
/quote KICK #dev bob :bye
# Esperado: bob é removido do #dev
# bob recebe notificação
```

---

### ✅ FASE 7: Comandos de Operador

**Objetivo**: Validar implementação dos 5 comandos obrigatórios.

#### Teste 7.1 - KICK
```bash
# irssi 1 (operadora)
/quote KICK #dev bob mensagem de saída
# Esperado: bob sai do canal e recebe razão
```

#### Teste 7.2 - INVITE
```bash
# irssi 1 (operadora, canal em modo +i)
/quote MODE #privado +i
/quote INVITE bob #privado
# bob
/join #privado
# Esperado: bob consegue entrar apenas com convite
```

#### Teste 7.3 - TOPIC
```bash
# irssi 1 (operadora)
/topic #dev Tópico do Canal de Desenvolvimento

# irssi 2
/join #dev
# Esperado: Vê o tópico

# Teste: Usuário regular tenta mudar (modo +t ativo)
/topic #dev novo tópico
# Esperado: Erro, apenas operadores
```

#### Teste 7.4 - MODE
```bash
# irssi 1 (operadora)

# Modo +i (invite-only)
/quote MODE #dev +i
# Esperado: Canal fica apenas para convidados

# Modo +k (key/senha)
/quote MODE #dev +k senha123
# Esperado: Outro cliente precisa /join #dev senha123

# Modo +l (limit)
/quote MODE #dev +l 5
# Esperado: Máximo 5 usuários no canal

# Modo +o (dar operador)
/quote MODE #dev +o alice
# Esperado: alice vira operadora

# Modo -o (remover operador)
/quote MODE #dev -o alice
# Esperado: alice deixa de ser operadora

# Modo +t (topic restriction)
/quote MODE #dev +t
# Esperado: /topic #dev novo_topico falha para usuários regulares
```

---

## 🔧 Comandos irssi Úteis

### Navegação
```
/window list              # Lista todas as abas abertas
/window <número>          # Muda para aba
Ctrl+P / Ctrl+N           # Navega entre abas
/query nick               # Abre conversa privada
```

### Informações
```
/whois nick               # Informações sobre usuário
/list                     # Lista canais disponíveis
/quote USERS              # Lista usuários conectados
/who #canal               # Lista usuários do canal
/quote NAMES #canal       # Idem
```

### Testes de Protocolo
```
/quote COMMAND args       # Envia comando IRC bruto
/debug ON                 # Ativa debug mode
/set autolog ON           # Salva logs
```

### Troubleshooting
```
/version                  # Versão do irssi
/quote PING               # Testa conexão (ping/pong)
/connect                  # Reconectar
/disconnect               # Desconectar
```

---

## 📝 Script de Teste Automatizado

Você pode criar um script para testar rapidamente:

### `test_server.sh`
```bash
#!/bin/bash

PORT=6667
PASSWORD="senha123"

echo "=== Teste 1: Conectividade ==="
(echo -e "NICK test\r\nQUIT\r\n" | nc localhost $PORT) &
sleep 0.5

echo "=== Teste 2: Autenticação ==="
(echo -e "PASS $PASSWORD\r\nNICK alice\r\nUSER alice 0 * :Alice\r\nQUIT\r\n" | nc localhost $PORT) &
sleep 0.5

echo "=== Teste 3: JOIN ==="
(echo -e "PASS $PASSWORD\r\nNICK bob\r\nUSER bob 0 * :Bob\r\nJOIN #test\r\nQUIT\r\n" | nc localhost $PORT) &
sleep 0.5

echo "Testes concluídos!"
```

Executar:
```bash
chmod +x test_server.sh
./test_server.sh
```

---

## 🎯 Checklist de Validação com irssi

- [ ] **Conectividade**: irssi conecta ao servidor
- [ ] **Autenticação**: Senha correta permite entrada
- [ ] **Nickname**: Pode definir e mudar nickname
- [ ] **Username**: Campo username funciona
- [ ] **JOIN**: Consegue entrar em canais
- [ ] **PRIVMSG em canal**: Mensagens aparecem para todos
- [ ] **PRIVMSG privado**: Mensagens privadas funcionam
- [ ] **KICK**: Operador consegue remover usuário
- [ ] **INVITE**: Convites funcionam em canais +i
- [ ] **TOPIC**: Tópico de canal funciona
- [ ] **MODE +i**: Canal invite-only funciona
- [ ] **MODE +k**: Senha do canal funciona
- [ ] **MODE +l**: Limite de usuários funciona
- [ ] **MODE +o**: Dar/remover operador funciona
- [ ] **MODE +t**: Restrição de tópico funciona
- [ ] **Múltiplas conexões**: Sem travar com 10+ usuários
- [ ] **Flood**: Aguenta spam sem crash
- [ ] **Desconexão abrupta**: Servidor continua operacional

---

## 🚨 Troubleshooting Comum

### Problema: "Connection refused"
**Solução**: Verifique se servidor está rodando na porta correta
```bash
netstat -tuln | grep 6667
```

### Problema: "Bad password"
**Solução**: Use `/quote PASS senha_correta`

### Problema: Mensagens não aparecem
**Solução**: 
1. Verifique se ambos clientes estão no mesmo canal
2. Use `/who #canal` para confirmar presença
3. Tente `/quote DEBUG ON` para ver pacotes

### Problema: irssi não mostra notificações
**Solução**: Configure notificações
```
/set visible_nicknames ON
/set activity_hide_targets #canal
```

---

## 💡 Dica de Eficiência

Para testes rápidos **sem usar irssi**:

```bash
# Teste com nc em um comando
(echo -e "PASS senha123\r\nNICK alice\r\nUSER alice 0 * :Alice\r\nJOIN #test\r\nPRIVMSG #test :oi\r\nQUIT\r\n"; sleep 1) | nc localhost 6667
```

Use `nc` para testes de edge cases (desconexão abrupta, comandos fragmentados).
Use `irssi` para testes de funcionalidade completa e modo interativo.

---

## 📚 Referências

- [irssi Documentation](https://irssi.org/)
- [RFC 1459 - IRC Protocol](https://tools.ietf.org/html/rfc1459)
- [IRC Commands](https://www.ircnumerics.org/

---

## Próximas Etapas

1. **Setup inicial**: Instale irssi
2. **Validação de conectividade**: Teste FASE 1
3. **Desenvolvimento incremental**: Complete FASE por FASE
4. **Testes finais**: Execute todas as FASES
5. **Teste de robustez**: Flood, desconexões, edge cases
