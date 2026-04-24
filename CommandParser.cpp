#include "CommandParser.hpp"
#include <iostream>

CommandParser::CommandParser() : _buffer("") {
}

CommandParser::~CommandParser() {
}

void CommandParser::appendData(const std::string &data) {
	_buffer += data;
}

bool CommandParser::hasCompleteCommand() const {
	// Um comando completo termina com \r\n ou \n
	return _buffer.find("\r\n") != std::string::npos || 
	       _buffer.find("\n") != std::string::npos;
}

std::string CommandParser::extractCommand() {
	size_t pos = _buffer.find("\r\n");
	
	// Se não encontrar \r\n, tenta apenas \n
	if (pos == std::string::npos) {
		pos = _buffer.find("\n");
	}

	// Se ainda não encontrou, retorna string vazia
	if (pos == std::string::npos) {
		return "";
	}

	// Extrai comando até o delimitador
	std::string command = _buffer.substr(0, pos);

	// Remove \r se estiver no final (caso de \r\n)
	if (!command.empty() && command[command.size() - 1] == '\r') {
		command.erase(command.size() - 1);
	}

	// Remove o comando + delimitador do buffer
	// Aumenta pos em 1 ou 2 dependendo se foi \r\n ou \n
	size_t delimiterLen = (_buffer[pos] == '\r') ? 2 : 1;
	_buffer.erase(0, pos + delimiterLen);

	return command;
}

std::string CommandParser::getBuffer() const {
	return _buffer;
}

void CommandParser::clearBuffer() {
	_buffer.clear();
}
