#!/bin/bash

# Script mais agressivo: dois clientes persistentes
# Client 1: envia comando e fecha
# Client 2: envia comando APÓS Client 1 fechar
# Verifica se Server responde a Client 2

cd "$(dirname "$0")/.."

echo "=== S0: Teste Agressivo - Clientes Simultâneos ==="
echo ""

if [ ! -f "ircserv" ]; then
    c++ -Wall -Wextra -Werror -std=c++98 -o ircserv Server.cpp Client.cpp main.cpp
fi

# Iniciar servidor
./ircserv 6667 test123 > /tmp/server.log 2>&1 &
SERVER_PID=$!
sleep 1

cleanup() {
    kill $SERVER_PID 2>/dev/null || true
    sleep 0.5
}
trap cleanup EXIT

echo "[*] Teste: Cliente 1 envia comando e desconecta, Cliente 2 enviará depois"

# Cliente 1: enviar NICK e desconectar
exec 3<>/dev/tcp/localhost/6667
echo "NICK alice" >&3
echo "USER alice 0 * :Alice" >&3
sleep 0.2
exec 3>&-  # fecha descriptor
sleep 0.5

echo "[*] Cliente 1 desconectou, agora Cliente 2 tentará conectar..."

# Cliente 2: conectar APÓS cliente 1 fechar
exec 4<>/dev/tcp/localhost/6667
echo "NICK bob" >&4
sleep 0.2

# Tentar ler resposta de Client 2
read -t 2 response <&4
if [ -n "$response" ]; then
    echo "✓ Cliente 2 recebeu resposta: $response"
else
    echo "✗ Cliente 2 NÃO recebeu resposta (SERVER BLOQUEADO?)"
fi
exec 4>&-

sleep 1

echo ""
echo "=== Análise de Logs ==="
grep "disconnected\|Clientes ativos" /tmp/server.log | head -10

# Verificar se server processou ambos clientes
if grep -q "NICK alice" /tmp/server.log && grep -q "NICK bob" /tmp/server.log; then
    echo "✓ Ambos comandos foram processados"
else
    echo "✗ Nem todos os comandos foram processados"
fi
