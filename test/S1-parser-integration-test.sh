#!/bin/bash

###############################################################################
# S1-PARSER-INTEGRATION-TEST.SH
#
# Testes de integração: CommandParser + Client + Server
# Valida comportamento em cenários reais
###############################################################################

cd /home/scr1b3s/cric

echo "═════════════════════════════════════════"
echo "S1 - PARSER INTEGRATION TESTS"
echo "═════════════════════════════════════════"
echo ""

# Recompile
c++ -Wall -Wextra -Werror -std=c++98 -o ircserv *.cpp 2>/dev/null || {
    echo "✗ Compilação falhou"
    exit 1
}

# TEST 1: Servidor recebe comando simples
echo -n "T1: Servidor recebe comando simples... "
timeout 3 ./ircserv 6667 test123 > /tmp/server_t1.log 2>&1 &
SRV=$!
sleep 1

echo -e "NICK alice\r\n" | timeout 1 nc localhost 6667 > /dev/null 2>&1
RESULT=$(grep -c "Executando comando: \[NICK alice\]" /tmp/server_t1.log || echo 0)

kill $SRV 2>/dev/null || true
wait $SRV 2>/dev/null || true

if [ "$RESULT" -eq 1 ]; then
    echo "✓"
else
    echo "✗"
fi

# TEST 2: Servidor recebe comando fragmentado
echo -n "T2: Servidor recebe comando fragmentado... "
timeout 3 ./ircserv 6667 test123 > /tmp/server_t2.log 2>&1 &
SRV=$!
sleep 1

# Enviar "NICK alice\r\n" em 2 partes
{
    printf "NICK al"
    sleep 0.3
    printf "ice\r\n"
    sleep 0.5
} | timeout 2 nc localhost 6667 > /dev/null 2>&1

RESULT=$(grep -c "Executando comando: \[NICK alice\]" /tmp/server_t2.log || echo 0)

kill $SRV 2>/dev/null || true
wait $SRV 2>/dev/null || true

if [ "$RESULT" -eq 1 ]; then
    echo "✓"
else
    echo "✗ (não encontrou comando processado)"
fi

# TEST 3: Servidor recebe múltiplos comandos
echo -n "T3: Servidor recebe múltiplos comandos... "
timeout 3 ./ircserv 6667 test123 > /tmp/server_t3.log 2>&1 &
SRV=$!
sleep 1

echo -e "NICK alice\r\nUSER alice 0 * :Alice\r\nPASS test123\r\n" | timeout 1 nc localhost 6667 > /dev/null 2>&1

RESULT_NICK=$(grep -c "Executando comando: \[NICK alice\]" /tmp/server_t3.log || echo 0)
RESULT_USER=$(grep -c "Executando comando: \[USER alice 0" /tmp/server_t3.log || echo 0)
RESULT_PASS=$(grep -c "Executando comando: \[PASS test123\]" /tmp/server_t3.log || echo 0)

kill $SRV 2>/dev/null || true
wait $SRV 2>/dev/null || true

if [ "$RESULT_NICK" -eq 1 ] && [ "$RESULT_USER" -eq 1 ] && [ "$RESULT_PASS" -eq 1 ]; then
    echo "✓"
else
    echo "✗ (NICK=$RESULT_NICK, USER=$RESULT_USER, PASS=$RESULT_PASS)"
fi

# TEST 4: Múltiplos clientes mantêm buffers independentes
echo -n "T4: Múltiplos clientes com buffers independentes... "
timeout 5 ./ircserv 6667 test123 > /tmp/server_t4.log 2>&1 &
SRV=$!
sleep 1

# Cliente 1: fragmentado
{
    printf "NICK "
    sleep 0.2
    printf "alice\r\n"
    sleep 1
} | timeout 3 nc localhost 6667 > /dev/null 2>&1 &

sleep 0.5

# Cliente 2: simples
echo -e "NICK bob\r\n" | timeout 1 nc localhost 6667 > /dev/null 2>&1

sleep 1

kill $SRV 2>/dev/null || true
wait $SRV 2>/dev/null || true

RESULT_A=$(grep -c "Executando comando: \[NICK alice\]" /tmp/server_t4.log || echo 0)
RESULT_B=$(grep -c "Executando comando: \[NICK bob\]" /tmp/server_t4.log || echo 0)

if [ "$RESULT_A" -eq 1 ] && [ "$RESULT_B" -eq 1 ]; then
    echo "✓"
else
    echo "✗ (alice=$RESULT_A, bob=$RESULT_B)"
fi

# TEST 5: Comando com TRAILING preservado
echo -n "T5: Comando com TRAILING preservado... "
timeout 3 ./ircserv 6667 test123 > /tmp/server_t5.log 2>&1 &
SRV=$!
sleep 1

echo -e "PRIVMSG #ch :hello world\r\n" | timeout 1 nc localhost 6667 > /dev/null 2>&1

RESULT=$(grep -c "Executando comando: \[PRIVMSG #ch :hello world\]" /tmp/server_t5.log || echo 0)

kill $SRV 2>/dev/null || true
wait $SRV 2>/dev/null || true

if [ "$RESULT" -eq 1 ]; then
    echo "✓"
else
    echo "✗"
fi

echo ""
echo "═════════════════════════════════════════"
echo "✅ INTEGRATION TESTS COMPLETO"
echo "═════════════════════════════════════════"
exit 0
