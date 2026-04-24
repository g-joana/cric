#include "Server.hpp"
#include <iostream>
#include <cstdlib>
#include <cctype>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>

Server::Server(int port, const std::string &password)
	: _port(port), _password(password), _fd(-1) {
	_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (_fd == -1)
		throw std::runtime_error("Failed to create socket");

	int opt = 1;	
	setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	struct sockaddr_in addr;
	addr.sin_family = AF_INET; // IPv4
	addr.sin_port = htons(_port); // Port number in network byte order
	addr.sin_addr.s_addr = INADDR_ANY; // Listen on all interfaces

	// Bind the socket to the address and port
	if (bind(_fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
		close(_fd);
		throw std::runtime_error("Failed to bind socket");
	}
	// Listen for incoming connections
	if (listen(_fd, 10) == -1) {
		close(_fd);
		throw std::runtime_error("Failed to listen on socket");
	}
	// Set the socket to non-blocking mode to avoid blocking the main thread
	if (fcntl(_fd, F_SETFL, O_NONBLOCK) == -1) {
		close(_fd);
		throw std::runtime_error("Failed to set non-blocking mode");
	}

	struct pollfd pfd;
	pfd.fd = _fd;
	pfd.events = POLLIN;
	pfd.revents = 0;
	_pollfds.push_back(pfd);

	std::cout << "Server listening on port " << _port << std::endl;
}

Server::~Server() {
	for (size_t i = 0; i < _pollfds.size(); i++)
		close(_pollfds[i].fd);

	// Clean up clients
	for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
		delete it->second;
	}
	_clients.clear();
}

void Server::_acceptClient() {
	struct sockaddr_in clientAddr;
	socklen_t clientLen = sizeof(clientAddr);
	int clientFd = accept(_fd, (struct sockaddr *)&clientAddr, &clientLen);
	if (clientFd == -1)
		return;
	// Set the client socket to non-blocking mode to avoid blocking the main thread
	if (fcntl(clientFd, F_SETFL, O_NONBLOCK) == -1) {
		close(clientFd);
		return;
	}

	_clients[clientFd] = new Client(clientFd);
	_pendingConnections.push_back(clientFd);

	std::cout << "New client connected: fd " << clientFd << std::endl;
}

void Server::_processPendingConnections() {
	// Add all pending connections to the poll list
	for (size_t i = 0; i < _pendingConnections.size(); i++) {
		int clientFd = _pendingConnections[i];
		struct pollfd pfd;
		pfd.fd = clientFd;
		pfd.events = POLLIN;
		pfd.revents = 0;
		_pollfds.push_back(pfd);
	}
	_pendingConnections.clear();
}

void Server::_removeClient(int fd) {
	// Remove from clients map
	std::map<int, Client*>::iterator it = _clients.find(fd);
	if (it != _clients.end()) {
		delete it->second;
		_clients.erase(it);
	}

	// Close the fd
	close(fd);

	// Remove from poll list
	for (size_t i = 0; i < _pollfds.size(); i++) {
		if (_pollfds[i].fd == fd) {
			_pollfds.erase(_pollfds.begin() + i);
			break;
		}
	}
}

bool Server::_isNickDuplicate(const std::string &nick) const {
	for (std::map<int, Client*>::const_iterator it = _clients.begin(); 
	     it != _clients.end(); ++it) {
		if (it->second->getNickname() == nick && !nick.empty()) {
			return true;
		}
	}
	return false;
}

void Server::_handlePASS(Client *client, const std::string &params) {
	// params = "PASS password" - need to extract password after "PASS "
	std::string::size_type pos = params.find(' ');
	if (pos == std::string::npos || pos + 1 >= params.length()) {
		// Invalid format
		std::string error = ":server 461 * PASS :Not enough parameters\r\n";
		send(client->getFd(), error.c_str(), error.length(), 0);
		return;
	}

	std::string password = params.substr(pos + 1);

	if (password == _password) {
		client->setIsAuthenticated(true);
		client->setState(AUTH);
		// RFC: PASS no retorna resposta positiva
		std::cout << "Client fd " << client->getFd() << " authenticated with PASS" << std::endl;
	} else {
		std::string error = ":server 464 * :Incorrect password\r\n";
		send(client->getFd(), error.c_str(), error.length(), 0);
		_removeClient(client->getFd());
		std::cout << "Client fd " << client->getFd() << " rejected due to wrong password" << std::endl;
	}
}

void Server::_handleNICK(Client *client, const std::string &params) {
	// params = "NICK nickname" - extract after "NICK "
	std::string::size_type pos = params.find(' ');
	if (pos == std::string::npos || pos + 1 >= params.length()) {
		std::string error = ":server 461 * NICK :Not enough parameters\r\n";
		send(client->getFd(), error.c_str(), error.length(), 0);
		return;
	}

	std::string nick = params.substr(pos + 1);

	// Trim trailing whitespace
	while (!nick.empty() && (nick[nick.length() - 1] == ' ' || nick[nick.length() - 1] == '\t')) {
		nick = nick.substr(0, nick.length() - 1);
	}

	// Validate: not empty, no spaces
	if (nick.empty()) {
		std::string error = ":server 461 * NICK :Not enough parameters\r\n";
		send(client->getFd(), error.c_str(), error.length(), 0);
		return;
	}

	if (nick.find(' ') != std::string::npos) {
		std::string error = ":server 461 * NICK :Invalid nickname\r\n";
		send(client->getFd(), error.c_str(), error.length(), 0);
		return;
	}

	// Check for duplicate
	if (_isNickDuplicate(nick)) {
		std::string reply = ":server 433 * " + nick + " :Nickname is already in use\r\n";
		send(client->getFd(), reply.c_str(), reply.length(), 0);
		return;
	}

	// OK - set nickname
	client->setNickname(nick);
	client->setHasNick(true);

	// Send confirmation
	std::string reply = ":server NICK " + nick + "\r\n";
	send(client->getFd(), reply.c_str(), reply.length(), 0);

	// Check if ready to register
	if (client->canTransitionToREGISTERED()) {
		client->setIsRegistered(true);
		client->setState(REGISTERED);
		_sendWelcome(client);
	} else {
		client->setState(ID);
	}

	std::cout << "Client fd " << client->getFd() << " set nickname: " << nick << std::endl;
}

void Server::_handleUSER(Client *client, const std::string &params) {
	// params = "USER username mode unused :realname"
	// Example: "USER alice 0 * :Alice Smith"

	std::string::size_type colonPos = params.find(':');
	if (colonPos == std::string::npos) {
		// No realname
		std::string error = ":server 461 * USER :Not enough parameters\r\n";
		send(client->getFd(), error.c_str(), error.length(), 0);
		return;
	}

	std::string realname = params.substr(colonPos + 1);

	// Extract username (word after "USER ")
	std::string::size_type start = params.find(' ');
	if (start == std::string::npos) {
		std::string error = ":server 461 * USER :Not enough parameters\r\n";
		send(client->getFd(), error.c_str(), error.length(), 0);
		return;
	}
	start++;

	std::string::size_type end = params.find(' ', start);
	if (end == std::string::npos) {
		std::string error = ":server 461 * USER :Not enough parameters\r\n";
		send(client->getFd(), error.c_str(), error.length(), 0);
		return;
	}

	std::string username = params.substr(start, end - start);

	if (username.empty() || realname.empty()) {
		std::string error = ":server 461 * USER :Not enough parameters\r\n";
		send(client->getFd(), error.c_str(), error.length(), 0);
		return;
	}

	// Set user data
	client->setUser(username);
	client->setRealname(realname);
	client->setHasUser(true);

	// Check if ready to register
	if (client->canTransitionToREGISTERED()) {
		client->setIsRegistered(true);
		client->setState(REGISTERED);
		_sendWelcome(client);
	}

	std::cout << "Client fd " << client->getFd() << " set user: " << username << " (" << realname << ")" << std::endl;
}

void Server::_sendWelcome(Client *client) {
	if (!client->getIsRegistered())
		return;

	std::string nick = client->getNickname();
	std::string reply = ":server 001 " + nick + " :Welcome to ft_irc\r\n";
	send(client->getFd(), reply.c_str(), reply.length(), 0);

	std::cout << "Client fd " << client->getFd() << " (" << nick << ") welcomed" << std::endl;
}

void Server::_processCommand(Client *client, const std::string &command) {
	if (command.empty())
		return;

	// Extract command name (first word)
	std::string::size_type spacePos = command.find(' ');
	std::string commandName;
	std::string params;

	if (spacePos == std::string::npos) {
		commandName = command;
		params = command;
	} else {
		commandName = command.substr(0, spacePos);
		params = command;
	}

	// Convert to uppercase for comparison
	for (size_t i = 0; i < commandName.length(); i++) {
		if (commandName[i] >= 'a' && commandName[i] <= 'z')
			commandName[i] = commandName[i] - 'a' + 'A';
	}

	// Dispatch to handlers
	if (commandName == "PASS") {
		_handlePASS(client, params);
	} else if (commandName == "NICK") {
		_handleNICK(client, params);
	} else if (commandName == "USER") {
		_handleUSER(client, params);
	} else if (commandName == "PING") {
		_handlePING(client, params);
	} else {
		// Unknown command - for now, silently ignore (will implement more commands in S3+)
		std::cout << "Client fd " << client->getFd() << " sent unknown command: " << commandName << std::endl;
	}
}

void Server::run() {
	while (true) {
		// Poll the sockets for events
		std::cout << "Waiting on poll.. Active clients: " << _pollfds.size() << std::endl;
		int ready = poll(&_pollfds[0], _pollfds.size(), -1);
		if (ready == -1)
			break;
		
		// Use index-based loop with special handling for erasure
		for (size_t i = 0; i < _pollfds.size(); ) {
			if (_pollfds[i].revents & POLLIN) {
				if (_pollfds[i].fd == _fd) {
					// Server socket - accept new connection
					_acceptClient();
					i++;
				}
				else {
					// Client socket - read data
					char buffer[1024];
					int bytesRead = recv(_pollfds[i].fd, buffer, sizeof(buffer) - 1, 0);

					if (bytesRead <= 0) {
						// Client disconnected
						int fdToRemove = _pollfds[i].fd;
						std::cout << "Client fd " << fdToRemove << " disconnected!" << std::endl;
						
						// Clean up client data
						std::map<int, Client*>::iterator it = _clients.find(fdToRemove);
						if (it != _clients.end()) {
							delete it->second;
							_clients.erase(it);
						}
						
						close(fdToRemove);
						
						// Remove from poll list using erase
						_pollfds.erase(_pollfds.begin() + i);
						// Don't increment i - erase already points to the next element
					}
					else {
						// Process received data
						buffer[bytesRead] = '\0';
						std::cout << "Data received from fd " << _pollfds[i].fd << " : " << buffer << std::endl;
						
						Client *client = _clients[_pollfds[i].fd];
						client->appendToBuffer(buffer);

						// Extract and process all complete commands
						while (client->hasCompleteCommand()) {
							std::string command = client->extractCommand();
							if (!command.empty()) {
								std::cout << "Processing command: [" << command << "]" << std::endl;
								_processCommand(client, command);
							}
						}
						i++;
					}
				}
			}
			else {
				i++;
			}
		}
		
		// Process any pending connections (adds them to poll list safely)
		_processPendingConnections();
	}
}

void Server::_handlePING(Client *client, const std::string &args) {
	client->sendMessage("PONG " + args);
}
