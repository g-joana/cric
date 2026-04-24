#ifndef COMMANDPARSER_HPP
# define COMMANDPARSER_HPP

# include <string>
# include <vector>

/**
 * CommandParser - Agregador de pacotes IRC fragmentados
 * 
 * Responsabilidades:
 * - Manter buffer de dados recebidos
 * - Detectar comandos completos (terminados com \r\n)
 * - Extrair um comando por vez
 * - Preservar dados não-processados para próximo ciclo
 */
class CommandParser {
private:
	std::string _buffer;

public:
	CommandParser();
	~CommandParser();

	/**
	 * Agrega dados recebidos do socket ao buffer
	 * Dados fragmentados são preservados para processamento futuro
	 */
	void appendData(const std::string &data);

	/**
	 * Verifica se há um comando completo no buffer
	 * Um comando completo termina com \r\n
	 */
	bool hasCompleteCommand() const;

	/**
	 * Extrai o próximo comando completo do buffer
	 * Remove \r\n do final
	 * Mantém dados restantes no buffer
	 */
	std::string extractCommand();

	/**
	 * Retorna o conteúdo atual do buffer (para debug)
	 */
	std::string getBuffer() const;

	/**
	 * Limpa completamente o buffer
	 */
	void clearBuffer();
};

#endif
