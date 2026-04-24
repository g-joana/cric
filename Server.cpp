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

	_clients[clientFd] = new Client(clientFd); // cria a lista de clientssss

	// Add the client socket to the pollfd list to be polled
	struct pollfd pfd;
	pfd.fd = clientFd;
	pfd.events = POLLIN;
	pfd.revents = 0;
	_pollfds.push_back(pfd);

	std::cout << "New client connected: fd " << clientFd << std::endl;
}

void Server::run() {
	while (true) {
		// Poll the sockets for events
		int ready = poll(&_pollfds[0], _pollfds.size(), -1);
		if (ready == -1)
			break;
		
		for (size_t i = 0; i < _pollfds.size(); i++) {
			if (_pollfds[i].revents & POLLIN) {
				if (_pollfds[i].fd == _fd)
				_acceptClient();
				else {
					char buffer[1024]; //safe size char
					int bytesReads = recv(_pollfds[i].fd, buffer, sizeof(buffer) -1, 0);

					if (bytesReads <= 0){ 
						int fdToRemove = _pollfds[i].fd;
						std::cout << "Client fd " << _pollfds[i].fd << " disconnected!" <<std::endl;
						std::map<int, Client*>::iterator it = _clients.find(fdToRemove);
						if (it != _clients.end()){
							delete it->second;
							_clients.erase(it);
						}
						
						std::cout << "removing fd " << fdToRemove << std::endl;
						close(fdToRemove);
						std::cout << "remove" << std::endl;

						_pollfds.erase(_pollfds.begin() + i);
						i--;

						continue; //forcar looping voltar para o topo novamente, ajuste para que depois de retirar um fd a proxima interacao respeite o novo estado do vetor
					}
					else{
						buffer[bytesReads] = '\0';
						std::cout << "Data recived from fd " << _pollfds[i].fd << " : " << buffer << std::endl;
						Client *c = _clients[_pollfds[i].fd];
						c->appendToBuffer(buffer);

						std::string clientBuffer = c->getBuffer();
						
						size_t pos;
						
						while((pos = clientBuffer.find("\n")) != std::string::npos){
							std::string command = clientBuffer.substr(0, pos);
							if (!command.empty() && command[command.size() - 1] == '\r')
								command.erase(command.size() - 1);
							if (!command.empty())
							{
								std::cout << "Executando comando: [" << command << "]" << std::endl; 
								//todo: implementar comandos pass, nick, ..
							}
							clientBuffer.erase(0, pos + 1);
							c->clearBuffer();
							c->appendToBuffer(clientBuffer);
						}

					}
				}
			}
		}
	}
}
