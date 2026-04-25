# Manual de Teste ft_irc com irssi

## Pré-requisitos
```bash
# Instalar irssi (se necessário)
sudo apt install irssi
```

## Testes Passo-a-Passo

### Terminal 1: Iniciar Servidor
```bash
cd /home/scr1b3s/cric
./ircserv 6667 password
```

### Terminal 2: Conectar com irssi
```bash
irssi
```

### Comandos irssi (Digitar em cada passo):
```
/connect localhost 6667
/quote PASS password
/nick alice
/quote USER alice 0 * :Alice User
```

## Teste T1: JOIN e PRIVMSG
```
/join #test
/msg #test Olá pessoal!
```
**Esperado**: Mensagem visível para todos em #test

## Teste T2: MODE +i (invite-only)
```
/join #secret
/mode #secret +i
/leave #secret
```
**Esperado**: Canal fica invite-only

### Novo cliente (Terminal 3):
```bash
irssi
/connect localhost 6667
/quote PASS password
/nick bob
/quote USER bob 0 * :Bob User
/join #secret
```
**Esperado**: `473 #secret :Cannot join channel (+i)` - NÃO deve entrar

### Voltar ao alice:
```
/invite bob #secret
```
**Esperado**: bob recebe INVITE

### bob tenta novamente:
```
/join #secret
```
**Esperado**: Agora consegue entrar

## Teste T3: MODE +t (topic restrito)
```
/join #topic
/mode #topic +t
```
**Esperado**: +t ativado

###bob (Terminal 3):
```
/join #topic
/topic #topic :Novo topico
```
**Esperado**: `482 #topic :You're not channel operator` - NÃO muda

### alice (muda):
```
/topic #topic :Topico Oficial
```
**Esperado**: Topic alterado

## Teste T4: MODE +k (password)
```
/join #private
/mode #private +k minha_senha
/leave #private
```
**Esperado**: Canal pede senha

### bob tenta:
```
/join #private
/join #private minima_senha
```
**Esperado**: Com senha errada - `475 #private :Cannot join channel (+k)`
```
/join #private minha_senha
```
**Esperado**: Entra com senha correta

## Teste T5: MODE +l (limite)
```
/join #limit
/mode #limit +l 2
```
**Esperado**: Limite 2 usuários

### Teste com 3 clientes:
- alice e bob #limit (2 usuários)
- carol tenta entrar
**Esperado**: `471 #limit :Cannot join channel (+l)`

## Teste T6: KICK
```
/kick #test bob
```
**Esperado**: bob é removido do canal

### Verificar:
```
/names #test
```
**Esperado**: bob não está na lista

## Teste T7: Operador
```
/op bob
```
**Esperado**: bob vira operador

```
/deop bob
```
**Esperado**: Remove operador

## Checklist Final

| Teste | Esperado | ✓/✗ |
|-------|---------|-------|
| JOIN #test | Entra | |
| PRIVMSG #test | Broadcast | |
| MODE +i | Bloqueia não-invited (473) | |
| MODE +t | Non-op não muda topic (482) | |
| MODE +k | Senha obrigatória (475) | |
| MODE +l | Limite respeitado (471) | |
| KICK | Remove miembro | |
| INVITE | Adiciona convite | |
| SIGINT (Ctrl+C) | Cleanup limpo | |

## Troubleshooting irssi

### Não conecta?
```
/disconnect
/connect localhost 6667
/quote PASS password
/nick seu_nick
/quote USER seu_nick 0 * :Seu Nome
```

### Ver debugging:
```
/set termsize 80
/wc
```

### Sai do irssi:
```
/quit
```