# Manual de Teste ft_irc com nc

## Serveidor
```bash
cd /home/scr1b3s/cric
./ircserv 6667 password
```

## Testes com nc

### Terminal 1: alice (operadora)
```bash
nc localhost 6667
PASS password
NICK alice
USER alice 0 * :Alice
JOIN #test
PRIVMSG #test :Ola pessoal!
QUIT
```

### Terminal 2: bob
```bash
nc localhost 6667
PASS password
NICK bob
USER bob 0 * :Bob
JOIN #test
PRIVMSG #test :Oi alice!
QUIT
```

---

## Teste Completo (copiar-colar)

### Servidor
```bash
./ircserv 6667 password
```

### Cliente 1 (alice)
```bash
printf "PASS password\r\nNICK alice\r\nUSER alice 0 * :Alice\r\nJOIN #test\r\n" | nc localhost 6667
```

### Cliente 2 (bob)
```bash
printf "PASS password\r\nNICK bob\r\nUSER bob 0 * :Bob\r\nJOIN #test\r\nPRIVMSG #test :Hello!\r\n" | nc localhost 6667
```

---

## Teste MODE +i

### Terminal 1: alice cria canal +i
```bash
nc localhost 6667
PASS password
NICK alice
USER alice 0 * :Alice
JOIN #secret
MODE #secret +i
QUIT
```

### Terminal 2: bob tenta entrar (FAIL)
```bash
nc localhost 6668
PASS password
NICK bob
USER bob 0 * :Bob
JOIN #secret
```
**Esperado**: `473 #secret :Cannot join channel (+i)`

---

## Teste MODE +k (senha)

```bash
# alice
printf "PASS password\r\nNICK alice\r\nUSER alice 0 * :Alice\r\nJOIN #private\r\nMODE #private +k minha_senha\r\n" | nc localhost 6667

# bob com senha ERRADA
printf "PASS password\r\nNICK bob\r\nUSER bob 0 * :Bob\r\nJOIN #private senha_errada\r\n" | nc localhost 6667
```
**Esperado**: `475 #private :Cannot join channel (+k)`

```bash
# bob com senha CORRETA
printf "PASS password\r\nNICK bob\r\nUSER bob 0 * :Bob\r\nJOIN #private minha_senha\r\n" | nc localhost 6667
```
**Esperado**: Entra no canal

---

## Teste MODE +l (limite)

```bash
# alice
printf "PASS password\r\nNICK alice\r\nUSER alice 0 * :Alice\r\nJOIN #limit\r\nMODE #limit +l 1\r\n" | nc localhost 6667

# bob tenta entrar (já tem alice)
printf "PASS password\r\nNICK bob\r\nUSER bob 0 * :Bob\r\nJOIN #limit\r\n" | nc localhost 6667
```
**Esperado**: `471 #limit :Cannot join channel (+l)`

---

## Teste KICK

```bash
# alice
printf "PASS password\r\nNICK alice\r\nUSER alice 0 * :Alice\r\nJOIN #kick\r\n" | nc localhost 6667

# alice kicka ela mesma
printf "KICK #kick alice\r\n" | nc localhost 6667
```

**Esperado**: Remove do canal

---

## Teste TOPIC

```bash
# alice
printf "PASS password\r\nNICK alice\r\nUSER alice 0 * :Alice\r\nJOIN #topic\r\nTOPIC #topic :Topico inicial\r\nMODE #topic +t\r\n" | nclocalhost 6667

# bob tenta mudar (FAIL)
printf "PASS password\r\nNICK bob\r\nUSER bob 0 * :Bob\r\nJOIN #topic\r\nTOPIC #topic :Novo topico\r\n" | nc localhost 6667
```
**Esperado**: `482 #topic :You're not channel operator`

---

## Checklist nc

| # | Teste | Comando | Esperado | ✓ |
|---|------|---------|---------|---|
| 1 | PASS | `PASS password` | OK | |
| 2 | NICK | `NICK alice` | OK | |
| 3 | USER | `USER alice 0 * :A` | 001 Welcome | |
| 4 | JOIN | `JOIN #test` | JOIN | |
| 5 | PRIVMSG | `PRIVMSG #test :msg` | Broadcast | |
| 6 | PART | `PART #test` | PART | |
| 7 | QUIT | `QUIT` | Desconecta | |
| 8 | MODE +i | `MODE #ch +i` | +i | |
| 9 | +i block | JOIN canal+i | 473 | |
| 10 | MODE +k | `MODE #ch +k s` | +k | |
| 11 | +k block | JOIN sem senha | 475 | |
| 12 | MODE +l | `MODE #ch +l 1` | +l | |
| 13 | +l block | 2 join | 471 | |
| 14 | KICK | `KICK #ch nick` | Remove | |
| 15 | TOPIC | `TOPIC #ch :t` | change | |
| 16 | +t block | Non-op TOPIC | 482 | |
| 17 | SIGINT | Ctrl+C | Cleanup | |

---

## Scripts de Teste Rápido

### flood.sh
```bash
#!/bin/bash
PORT=6667
PASS=password

./ircserv $PORT $PASS &
sleep 1

for i in {1..10}; do
  (printf "PASS $PASS\r\nNICK u$i\r\nUSER u$i 0 * :U$i\r\n" | nc localhost $PORT) &
done
sleep 2
echo "Teste flood OK"
```

### multiple.sh
```bash
#!/bin/bash
# 3 clientes simultâneos
for n in alice bob carol; do
  (printf "PASS p\r\nNICK $n\r\nUSER $n 0 * :$n\r\nJOIN #test\r\n" | nc localhost 6667) &
done
sleep 1
echo "Multiconexão OK"
```