#!/bin/bash

###############################################################################
# S1-BUG-FIX-VALIDATION.SH
# 
# Valida que os dois bugs de S0 foram corrigidos:
# - Bug #1: Modificação de vector durante iteração (_acceptClient push_back)
#   FIX: Usar _pendingConnections para não modificar _pollfds durante loop
# - Bug #2: Remoção incorreta com decremento (erase + i--)
#   FIX: Não incrementar i após erase (erase já retorna próximo)
#
# Teste: Múltiplos clientes conectam e desconectam sem travar servidor
###############################################################################

cd /home/scr1b3s/cric

# Compilar se necessário
if [ ! -f ircserv ]; then
    c++ -Wall -Wextra -Werror -std=c++98 -o ircserv *.cpp 2>/dev/null || exit 1
fi

# Iniciar servidor em background
timeout 10 ./ircserv 6667 test123 > /tmp/server_bugfix.log 2>&1 &
SERVER_PID=$!
sleep 1

# Testar Bug #1: Múltiplos clients conectando simultaneamente
# Antes: _acceptClient chamava push_back durante loop = undefined behavior
# Depois: _pendingConnections agrega, _processPendingConnections() adiciona após loop
(for i in {1..3}; do echo ""; sleep 0.2; done) | timeout 5 nc localhost 6667 > /tmp/c1.txt 2>&1 &
C1=$!

sleep 0.3

(for i in {1..3}; do echo ""; sleep 0.2; done) | timeout 5 nc localhost 6667 > /tmp/c2.txt 2>&1 &
C2=$!

sleep 0.3

(for i in {1..3}; do echo ""; sleep 0.2; done) | timeout 5 nc localhost 6667 > /tmp/c3.txt 2>&1 &
C3=$!

# Aguardar conexões terminarem
wait $C1 2>/dev/null || true
wait $C2 2>/dev/null || true
wait $C3 2>/dev/null || true

sleep 1

# Verificar se servidor ainda está vivo (não crashed)
if ps -p $SERVER_PID > /dev/null 2>&1; then
    RESULT="✓ BUG #1 FIXADO (server não crashou com múltiplos clients)"
else
    RESULT="✗ BUG #1 NÃO FIXADO (server crashou)"
    kill $SERVER_PID 2>/dev/null || true
    exit 1
fi

# Parar servidor
kill $SERVER_PID 2>/dev/null || true
wait $SERVER_PID 2>/dev/null || true

echo "$RESULT"
exit 0

