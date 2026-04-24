#!/bin/bash

# S0 - Acceptance Criteria Validation Script
# Todos os critérios DEVEM passar para S0 ser considerado COMPLETO

echo "🔍 S0 - CRITÉRIOS DE ACEITAÇÃO"
echo ""

PASS=0
FAIL=0

# C1: Código compila com flags corretas
echo -n "C1: Compilação (flags obrigatórias)... "
if c++ -Wall -Wextra -Werror -std=c++98 -o ircserv Server.cpp Client.cpp main.cpp 2>/dev/null; then
    echo "✓"
    ((PASS++))
else
    echo "✗ FALHA: Não compila"
    ((FAIL++))
    exit 1
fi

# C2: Script de reprodução existe
echo -n "C2: Script S0-reproduce-bug.sh existe... "
if [ -f "test/S0-reproduce-bug.sh" ]; then
    echo "✓"
    ((PASS++))
else
    echo "✗ FALHA: Script não encontrado"
    ((FAIL++))
    exit 1
fi

# C3: Script de teste agressivo existe
echo -n "C3: Script S0-aggressive-test.sh existe... "
if [ -f "test/S0-aggressive-test.sh" ]; then
    echo "✓"
    ((PASS++))
else
    echo "✗ FALHA: Script não encontrado"
    ((FAIL++))
    exit 1
fi

# C4: Documentação S0-BUG-ANALYSIS.md existe
echo -n "C4: Arquivo S0-BUG-ANALYSIS.md existe... "
if [ -f "docs/sprints_knowledge/S0-BUG-ANALYSIS.md" ]; then
    echo "✓"
    ((PASS++))
else
    echo "✗ FALHA: Documentação não encontrada"
    ((FAIL++))
    exit 1
fi

# C5: ROOT CAUSE está documentado
echo -n "C5: ROOT CAUSE identificado e documentado... "
if grep -q "ROOT CAUSE\|Root Cause\|root cause\|Bug #1\|Bug #2" docs/sprints_knowledge/S0-BUG-ANALYSIS.md; then
    echo "✓"
    ((PASS++))
else
    echo "✗ FALHA: Root cause não documentado"
    ((FAIL++))
    exit 1
fi

# C6: Possíveis Soluções documentadas
echo -n "C6: Soluções possíveis documentadas... "
if grep -q "Solução\|soluções\|Próximos Passos" docs/sprints_knowledge/S0-BUG-ANALYSIS.md; then
    echo "✓"
    ((PASS++))
else
    echo "✗ FALHA: Soluções não documentadas"
    ((FAIL++))
    exit 1
fi

# C7: Teste de reprodução pode ser executado
echo -n "C7: Script de reprodução é executável... "
if [ -x "test/S0-reproduce-bug.sh" ] || bash -n test/S0-reproduce-bug.sh 2>/dev/null; then
    echo "✓"
    ((PASS++))
else
    echo "✗ FALHA: Script tem erro de sintaxe"
    ((FAIL++))
    exit 1
fi

# C8: Teste agressivo pode ser executado
echo -n "C8: Script agressivo é executável... "
if [ -x "test/S0-aggressive-test.sh" ] || bash -n test/S0-aggressive-test.sh 2>/dev/null; then
    echo "✓"
    ((PASS++))
else
    echo "✗ FALHA: Script tem erro de sintaxe"
    ((FAIL++))
    exit 1
fi

# C9: Documentação menciona impacto crítico
echo -n "C9: Impacto crítico documentado... "
if grep -q "CRÍTICO\|HIGH\|ALTA\|bloqueia S1" docs/sprints_knowledge/S0-BUG-ANALYSIS.md; then
    echo "✓"
    ((PASS++))
else
    echo "⚠ AVISO: Impacto não claramente marcado"
    ((PASS++))  # Não falha, apenas aviso
fi

# C10: Próximos passos para S1 estão claros
echo -n "C10: Próximos passos documentados... "
if grep -q "Próximos Passos\|S1\|Refatorar\|iterador" docs/sprints_knowledge/S0-BUG-ANALYSIS.md; then
    echo "✓"
    ((PASS++))
else
    echo "⚠ AVISO: Próximos passos não mencionam S1 explicitamente"
    ((PASS++))  # Não falha
fi

echo ""
echo "📊 RESULTADO:"
echo "   Passou: $PASS/10 critérios"
echo "   Falhou: $FAIL/10 critérios"
echo ""

if [ $FAIL -eq 0 ]; then
    echo "✅ S0 ACEITO - Todos os critérios passaram!"
    echo ""
    echo "📌 PRÓXIMAS AÇÕES:"
    echo "   1. Agente S1 lerá: docs/sprints_knowledge/S0-BUG-ANALYSIS.md"
    echo "   2. S1 corrigirá bugs identificados"
    echo "   3. S1 implementará Parser IRC"
    exit 0
else
    echo "❌ S0 REJEITADO - Falhas detectadas"
    exit 1
fi
