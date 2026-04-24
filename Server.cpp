#include "Server.hpp"
#include <iostream>
#include <cstdlib>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>

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

void Server::run() {
	while (true) {
		// Poll the sockets for events
		std::cout << "Aguardando no poll.. Clientes ativos: " << _pollfds.size() << std::endl;
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
						
						// Remove from poll list using erase, which returns iterator to next element
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
								std::cout << "Executando comando: [" << command << "]" << std::endl;
								// TODO: implement PASS, NICK, USER, etc commands
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

