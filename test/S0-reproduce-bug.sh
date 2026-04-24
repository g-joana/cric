#!/bin/bash

set -e

# Script para reproduzir o bug: cliente com Ctrl+D bloqueia outros

cd "$(dirname "$0")/.."

echo "=== S0: Reprodução do Bug Ctrl+D ==="
echo ""

# Compilar se necessário
if [ ! -f "ircserv" ]; then
    echo "[*] Compilando servidor..."
    c++ -Wall -Wextra -Werror -std=c++98 -o ircserv Server.cpp Client.cpp main.cpp
fi

# Iniciar servidor
echo "[*] Iniciando servidor na porta 6667..."
./ircserv 6667 test123 > /tmp/server.log 2>&1 &
SERVER_PID=$!

# Esperar servidor inicializar
sleep 1

# Função para limpar no exit
cleanup() {
    kill $SERVER_PID 2>/dev/null || true
    sleep 0.5
}
trap cleanup EXIT

# Cliente 1: Enviar dados e depois fechar imediatamente (simular Ctrl+D)
echo "[*] Conectando Cliente 1..."
(
    sleep 0.2
    echo "NICK alice"
    sleep 0.2
) | nc -N localhost 6667 > /tmp/client1.log 2>&1 &
C1_PID=$!

sleep 0.5

# Cliente 2: Conectar e enviar command
echo "[*] Conectando Cliente 2..."
(
    sleep 0.2
    echo "NICK bob"
    sleep 1
) | nc -N localhost 6667 > /tmp/client2.log 2>&1 &
C2_PID=$!

# Esperar ambos terminarem
wait $C1_PID 2>/dev/null || true
wait $C2_PID 2>/dev/null || true

echo ""
echo "=== Logs ===" 
echo "--- Server Log ---"
head -20 /tmp/server.log

echo ""
echo "--- Client 1 Log ---"
cat /tmp/client1.log || echo "(vazio)"

echo ""
echo "--- Client 2 Log ---"
cat /tmp/client2.log || echo "(vazio)"

echo ""
echo "=== Resultado ===" 

# Analisar se o servidor continuou funcionando
if grep -q "disconnected" /tmp/server.log; then
    echo "✓ Servidor detectou desconexão"
else
    echo "⚠ Servidor não detectou desconexão"
fi

if grep -q "Client 2\|bob\|Clientes ativos" /tmp/server.log; then
    echo "✓ Servidor continuou processando clientes após Client 1 desconectar"
    echo ""
    echo "✓ BUG NÃO ENCONTRADO: Server mantém clientes conectados"
    exit 0
else
    echo "✗ Servidor NÃO processou Client 2 após Client 1 desconectar"
    echo ""
    echo "✗ BUG ENCONTRADO: Desconexão de um cliente bloqueia outros"
    exit 1
fi
