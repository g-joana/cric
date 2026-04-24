#!/bin/bash

###############################################################################
# test-parser-unit.cpp
# 
# Testes unitários para CommandParser
# Compila em standalone e valida cada caso de uso
###############################################################################

cat > /tmp/test_parser.cpp << 'EOF'
#include <iostream>
#include <cassert>
#include <string>

class CommandParser {
private:
    std::string _buffer;
public:
    CommandParser() : _buffer("") {}
    
    void appendData(const std::string &data) {
        _buffer += data;
    }
    
    bool hasCompleteCommand() const {
        return _buffer.find("\r\n") != std::string::npos || 
               _buffer.find("\n") != std::string::npos;
    }
    
    std::string extractCommand() {
        size_t pos = _buffer.find("\r\n");
        
        if (pos == std::string::npos) {
            pos = _buffer.find("\n");
        }
        
        if (pos == std::string::npos) {
            return "";
        }
        
        std::string command = _buffer.substr(0, pos);
        
        if (!command.empty() && command[command.size() - 1] == '\r') {
            command.erase(command.size() - 1);
        }
        
        size_t delimiterLen = (_buffer[pos] == '\r') ? 2 : 1;
        _buffer.erase(0, pos + delimiterLen);
        
        return command;
    }
    
    std::string getBuffer() const {
        return _buffer;
    }
};

int main() {
    int passed = 0;
    int failed = 0;
    
    // TEST 1: Comando simples com \r\n
    {
        CommandParser p;
        p.appendData("NICK alice\r\n");
        
        if (p.hasCompleteCommand()) {
            std::string cmd = p.extractCommand();
            if (cmd == "NICK alice") {
                std::cout << "✓ T1: Comando simples com \\r\\n" << std::endl;
                passed++;
            } else {
                std::cout << "✗ T1: Esperado 'NICK alice', recebeu '" << cmd << "'" << std::endl;
                failed++;
            }
        } else {
            std::cout << "✗ T1: hasCompleteCommand() retornou false" << std::endl;
            failed++;
        }
    }
    
    // TEST 2: Comando fragmentado em 2 partes
    {
        CommandParser p;
        p.appendData("NICK ali");
        
        if (!p.hasCompleteCommand()) {
            p.appendData("ce\r\n");
            if (p.hasCompleteCommand()) {
                std::string cmd = p.extractCommand();
                if (cmd == "NICK alice") {
                    std::cout << "✓ T2: Comando fragmentado em 2 partes" << std::endl;
                    passed++;
                } else {
                    std::cout << "✗ T2: Esperado 'NICK alice', recebeu '" << cmd << "'" << std::endl;
                    failed++;
                }
            } else {
                std::cout << "✗ T2: hasCompleteCommand() ainda false após 2ª parte" << std::endl;
                failed++;
            }
        } else {
            std::cout << "✗ T2: 1ª parte já devia não ter comando completo" << std::endl;
            failed++;
        }
    }
    
    // TEST 3: Múltiplos comandos em um append
    {
        CommandParser p;
        p.appendData("NICK alice\r\nUSER alice 0 * :Alice\r\n");
        
        if (p.hasCompleteCommand()) {
            std::string cmd1 = p.extractCommand();
            if (cmd1 != "NICK alice") {
                std::cout << "✗ T3a: 1º comando inválido" << std::endl;
                failed++;
            } else if (p.hasCompleteCommand()) {
                std::string cmd2 = p.extractCommand();
                if (cmd2 == "USER alice 0 * :Alice") {
                    std::cout << "✓ T3: Múltiplos comandos em um append" << std::endl;
                    passed++;
                } else {
                    std::cout << "✗ T3b: 2º comando inválido: '" << cmd2 << "'" << std::endl;
                    failed++;
                }
            } else {
                std::cout << "✗ T3c: 2º comando não detectado" << std::endl;
                failed++;
            }
        } else {
            std::cout << "✗ T3: hasCompleteCommand() false" << std::endl;
            failed++;
        }
    }
    
    // TEST 4: Comando com TRAILING (PRIVMSG)
    {
        CommandParser p;
        p.appendData("PRIVMSG #ch :hello world\r\n");
        
        if (p.hasCompleteCommand()) {
            std::string cmd = p.extractCommand();
            if (cmd == "PRIVMSG #ch :hello world") {
                std::cout << "✓ T4: Comando com TRAILING (PRIVMSG)" << std::endl;
                passed++;
            } else {
                std::cout << "✗ T4: Comando inválido: '" << cmd << "'" << std::endl;
                failed++;
            }
        } else {
            std::cout << "✗ T4: hasCompleteCommand() false" << std::endl;
            failed++;
        }
    }
    
    // TEST 5: Buffer residual após extrair
    {
        CommandParser p;
        p.appendData("NICK alice\r\nNICK");
        
        std::string cmd = p.extractCommand();
        std::string remaining = p.getBuffer();
        
        if (cmd == "NICK alice" && remaining == "NICK") {
            std::cout << "✓ T5: Buffer residual preservado" << std::endl;
            passed++;
        } else {
            std::cout << "✗ T5: cmd='" << cmd << "', buffer='" << remaining << "'" << std::endl;
            failed++;
        }
    }
    
    // TEST 6: Comando com apenas \n (sem \r)
    {
        CommandParser p;
        p.appendData("NICK alice\n");
        
        if (p.hasCompleteCommand()) {
            std::string cmd = p.extractCommand();
            if (cmd == "NICK alice") {
                std::cout << "✓ T6: Comando com apenas \\n" << std::endl;
                passed++;
            } else {
                std::cout << "✗ T6: Comando inválido: '" << cmd << "'" << std::endl;
                failed++;
            }
        } else {
            std::cout << "✗ T6: hasCompleteCommand() false" << std::endl;
            failed++;
        }
    }
    
    // TEST 7: Extract sem comando completo
    {
        CommandParser p;
        p.appendData("NICK alice");
        
        if (!p.hasCompleteCommand()) {
            std::string cmd = p.extractCommand();
            if (cmd == "") {
                std::cout << "✓ T7: extractCommand() sem \\r\\n retorna vazio" << std::endl;
                passed++;
            } else {
                std::cout << "✗ T7: Esperado vazio, recebeu '" << cmd << "'" << std::endl;
                failed++;
            }
        } else {
            std::cout << "✗ T7: hasCompleteCommand() should be false" << std::endl;
            failed++;
        }
    }
    
    // TEST 8: Múltiplas fragmentações
    {
        CommandParser p;
        p.appendData("NI");
        if (p.hasCompleteCommand()) {
            std::cout << "✗ T8a: hasCompleteCommand() true prematuramente" << std::endl;
            failed++;
        } else {
            p.appendData("CK al");
            if (p.hasCompleteCommand()) {
                std::cout << "✗ T8b: hasCompleteCommand() true antes de \\r\\n" << std::endl;
                failed++;
            } else {
                p.appendData("ice\r\n");
                if (p.hasCompleteCommand()) {
                    std::string cmd = p.extractCommand();
                    if (cmd == "NICK alice") {
                        std::cout << "✓ T8: Múltiplas fragmentações (3 partes)" << std::endl;
                        passed++;
                    } else {
                        std::cout << "✗ T8: Comando inválido: '" << cmd << "'" << std::endl;
                        failed++;
                    }
                } else {
                    std::cout << "✗ T8c: hasCompleteCommand() false após \\r\\n" << std::endl;
                    failed++;
                }
            }
        }
    }
    
    std::cout << std::endl;
    std::cout << "═════════════════════════════════" << std::endl;
    std::cout << "RESULTADOS: " << passed << " passaram, " << failed << " falharam" << std::endl;
    std::cout << "═════════════════════════════════" << std::endl;
    
    return (failed > 0) ? 1 : 0;
}
EOF

# Compilar e rodar
cd /tmp
c++ -Wall -Wextra -Werror -std=c++98 -o test_parser test_parser.cpp
./test_parser
