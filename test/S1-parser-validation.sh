#!/bin/bash

###############################################################################
# S1-PARSER-VALIDATION.SH
#
# Valida que CommandParser funciona corretamente em diferentes cenários:
# 1. Comando simples (NICK alice\r\n)
# 2. Comando fragmentado em múltiplas partes
# 3. Múltiplos comandos em um append
# 4. Comando com TRAILING (PRIVMSG)
# 5. Buffer residual preservado
# 6. Comando com \n (sem \r)
# 7. Edge case: extract sem \r\n
# 8. Múltiplas fragmentações (3+ partes)
###############################################################################

# Executar testes unitários
bash /home/scr1b3s/cric/test/run-parser-unit-tests.sh
exit $?

