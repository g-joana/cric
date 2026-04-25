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

	// Clean up channels
	for (std::map<std::string, Channel*>::iterator it = _channels.begin(); it != _channels.end(); ++it) {
		delete it->second;
	}
	_channels.clear();
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
	std::map<int, Client*>::iterator it = _clients.find(fd);
	if (it != _clients.end()) {
		Client *client = it->second;
		const std::set<std::string> &chans = client->getChannels();
		for (std::set<std::string>::const_iterator cit = chans.begin(); cit != chans.end(); ++cit) {
			Channel *channel = _findChannel(*cit);
			if (channel) {
				std::string quitMsg = ":" + client->getNickname()
				                + "!" + client->getUser()
				                + "@server QUIT :Gone";
				channel->broadcast(quitMsg);
				channel->removeMember(client->getFd());
				if (channel->getMemberCount() == 0) {
					_channels.erase(*cit);
					delete channel;
				}
			}
		}
		delete it->second;
		_clients.erase(it);
	}
	close(fd);
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

Client *Server::_findClientByNick(const std::string &nick) const {
	if (nick.empty())
		return NULL;
	for (std::map<int, Client*>::const_iterator it = _clients.begin();
	     it != _clients.end(); ++it) {
		if (it->second->getNickname() == nick)
			return it->second;
	}
	return NULL;
}

Channel *Server::_findChannel(const std::string &name) const {
	if (name.empty() || name[0] != '#')
		return NULL;
	std::map<std::string, Channel*>::const_iterator it = _channels.find(name);
	if (it != _channels.end())
		return it->second;
	return NULL;
}

void Server::_handlePASS(Client *client, const std::string &params) {
	// params = "password"
	std::string password = params;
	while (!password.empty() && (password[password.size()-1] == ' ' || password[password.size()-1] == '\t' || password[password.size()-1] == '\r' || password[password.size()-1] == '\n'))
		password.erase(password.size()-1);
	while (!password.empty() && (password[0] == ' ' || password[0] == '\t'))
		password.erase(0, 1);

	if (password.empty()) {
		std::string error = ":server 461 * PASS :Not enough parameters\r\n";
		send(client->getFd(), error.c_str(), error.length(), 0);
		return;
	}

	if (password == _password) {
		client->setIsAuthenticated(true);
		client->setState(AUTH);
		std::cout << "Client fd " << client->getFd() << " authenticated with PASS" << std::endl;
	} else {
		std::string error = ":server 464 * :Incorrect password\r\n";
		send(client->getFd(), error.c_str(), error.length(), 0);
		_removeClient(client->getFd());
		std::cout << "Client fd " << client->getFd() << " rejected due to wrong password" << std::endl;
	}
}

void Server::_handleNICK(Client *client, const std::string &params) {
	// params = "nickname"
	std::string nick = params;
	while (!nick.empty() && (nick[nick.size()-1] == ' ' || nick[nick.size()-1] == '\t' || nick[nick.size()-1] == '\r' || nick[nick.size()-1] == '\n'))
		nick.erase(nick.size()-1);
	while (!nick.empty() && (nick[0] == ' ' || nick[0] == '\t'))
		nick.erase(0, 1);

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
	// params = "username mode unused :realname"
	// Example: "alice 0 * :Alice Smith"

	std::string::size_type colonPos = params.find(':');
	if (colonPos == std::string::npos) {
		std::string error = ":server 461 * USER :Not enough parameters\r\n";
		send(client->getFd(), error.c_str(), error.length(), 0);
		return;
	}

	std::string realname = params.substr(colonPos + 1);

	std::string::size_type start = 0;
	while (start < params.size() && params[start] == ' ')
		start++;

	std::string::size_type end = params.find(' ', start);
	if (end == std::string::npos || end > colonPos) {
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

	std::string::size_type spacePos = command.find(' ');
	std::string commandName;
	std::string params;

	if (spacePos == std::string::npos) {
		commandName = command;
		params = "";
	} else {
		commandName = command.substr(0, spacePos);
		params = command.substr(spacePos + 1);
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
	} else if (commandName == "PRIVMSG") {
		_handlePRIVMSG(client, params);
	} else if (commandName == "JOIN") {
		_handleJOIN(client, params);
	} else if (commandName == "PART") {
		_handlePART(client, params);
	} else if (commandName == "QUIT") {
		_handleQUIT(client, params);
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
						
						// Get client for cleanup
						std::map<int, Client*>::iterator it = _clients.find(fdToRemove);
						Client *client = NULL;
						if (it != _clients.end()) {
							client = it->second;
							// Notify all channels the client was in
							const std::set<std::string> &chans = client->getChannels();
							for (std::set<std::string>::const_iterator ch = chans.begin(); ch != chans.end(); ++ch) {
								Channel *channel = _findChannel(*ch);
								if (channel) {
									std::string quitMsg = ":" + client->getNickname()
									                + "!" + client->getUser()
									                + "@server QUIT :Gone";
									channel->broadcast(quitMsg, client->getFd());
									channel->removeMember(client->getFd());
									if (channel->getMemberCount() == 0) {
										_channels.erase(*ch);
										delete channel;
									}
								}
							}
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
	std::string::size_type pos = args.find(' ');
	if (pos == std::string::npos) {
		client->sendMessage("PONG");
		return;
	}
	pos++;
	while (pos < args.size() && args[pos] == ' ')
		pos++;

	std::string token = args.substr(pos);
	client->sendMessage("PONG " + token);
}

void Server::_handlePRIVMSG(Client *client, const std::string &args) {
	if (!client->getIsRegistered()) {
		client->sendMessage(":server 451 * :You have not registered");
		return;
	}
	std::string::size_type pos = args.find(' ');
	if (pos == std::string::npos) {
		client->sendMessage(":server 411 " + client->getNickname() + " :No recipient given (PRIVMSG)");
		return;
	}
	pos++;
	while (pos < args.size() && args[pos] == ' ')
		pos++;

	if (pos >= args.size()) {
		client->sendMessage(":server 411 " + client->getNickname() + " :No recipient given (PRIVMSG)");
		return;
	}

	std::string target;
	std::string text;

	if (args[pos] == ':') {
		target = args.substr(0, pos - 1);
		text = args.substr(pos + 1);
	} else {
		std::string::size_type targetEnd = args.find(' ', pos);
		if (targetEnd == std::string::npos) {
			target = args.substr(pos);
			text = "";
		} else {
			target = args.substr(pos, targetEnd - pos);
			std::string::size_type textStart = targetEnd;
			while (textStart < args.size() && args[textStart] == ' ')
				textStart++;
			if (textStart >= args.size())
				text = "";
			else if (args[textStart] == ':')
				text = args.substr(textStart + 1);
			else
				text = args.substr(textStart);
		}
	}

	if (text.empty()) {
		client->sendMessage(":server 412 " + client->getNickname() + " :No text to send");
		return;
	}

	if (target[0] == '#') {
		Channel *channel = _findChannel(target);
		if (!channel) {
			client->sendMessage(":server 403 " + target + " :No such channel");
			return;
		}
		if (!channel->isMember(client->getFd())) {
			client->sendMessage(":server 442 " + target + " :You're not on that channel");
			return;
		}
		std::string msg = ":" + client->getNickname()
		                 + "!" + client->getUser()
		                 + "@server PRIVMSG " + target
		                 + " :" + text;
		channel->broadcast(msg, client->getFd());
		std::cout << "PRIVMSG " << client->getNickname() << " -> " << target
		          << " : " << text << std::endl;
		return;
	}

	Client *dest = _findClientByNick(target);
	if (!dest) {
		client->sendMessage(":server 401 " + client->getNickname() + " " + target + " :No such nick/channel");
		return;
	}
	std::string forwarded = ":" + client->getNickname()
 	                      + "!" + client->getUser()
 	                      + "@server PRIVMSG " + target
 	                      + " :" + text;
	dest->sendMessage(forwarded);

	std::cout << "PRIVMSG " << client->getNickname() << " -> " << target
 	          << " : " << text << std::endl;
}

void Server::_handleJOIN(Client *client, const std::string &args) {
	if (!client->getIsRegistered()) {
		client->sendMessage(":server 451 * :You have not registered");
		return;
	}

	std::string channelName;
	std::string key;

	std::string::size_type pos = args.find(' ');
	if (pos == std::string::npos) {
		channelName = args;
	} else {
		channelName = args.substr(0, pos);
		key = args.substr(pos + 1);
		while (key[0] == ' ')
			key = key.substr(1);
	}

	if (channelName.empty() || channelName[0] != '#') {
		client->sendMessage(":server 403 " + channelName + " :No such channel");
		return;
	}

	Channel *channel = _findChannel(channelName);
	if (!channel) {
		channel = new Channel(channelName);
		_channels[channelName] = channel;
	}

	channel->addMember(client);
	channel->addOperator(client->getFd());
	client->addChannel(channelName);

	std::string joinMsg = ":" + client->getNickname()
	                    + "!" + client->getUser()
	                    + "@server JOIN " + channelName;

	channel->broadcast(joinMsg, client->getFd());

	client->sendMessage(joinMsg);

	std::string topic = channel->getTopic();
	if (!topic.empty()) {
		client->sendMessage(":server 332 " + client->getNickname() + " " + channelName + " :" + topic);
	} else {
		client->sendMessage(":server 331 " + client->getNickname() + " " + channelName + " :No topic is set");
	}

	client->sendMessage(":server 353 " + client->getNickname() + " = " + channelName + " :@" + client->getNickname());
	client->sendMessage(":server 366 " + client->getNickname() + " " + channelName + " :End of /NAMES list");

	std::cout << "Client " << client->getNickname() << " joined " << channelName << std::endl;
}

void Server::_handlePART(Client *client, const std::string &args) {
	if (!client->getIsRegistered()) {
		client->sendMessage(":server 451 * :You have not registered");
		return;
	}

	std::string channelName;
	std::string::size_type pos = args.find(' ');
	if (pos == std::string::npos) {
		channelName = args;
	} else {
		channelName = args.substr(0, pos);
	}

	if (channelName.empty() || channelName[0] != '#') {
		client->sendMessage(":server 403 " + channelName + " :No such channel");
		return;
	}

	Channel *channel = _findChannel(channelName);
	if (!channel) {
		client->sendMessage(":server 403 " + channelName + " :No such channel");
		return;
	}

	if (!channel->isMember(client->getFd())) {
		client->sendMessage(":server 442 " + channelName + " :You're not on that channel");
		return;
	}

	channel->removeMember(client->getFd());
client->removeChannel(channelName);

	std::string partMsg = ":" + client->getNickname()
 	                   + "!" + client->getUser()
 	                   + "@server PART " + channelName;
	channel->broadcast(partMsg);

	client->sendMessage(partMsg);

	if (channel->getMemberCount() == 0) {
		delete channel;
		_channels.erase(channelName);
	}

	std::cout << "Client " << client->getNickname() << " left " << channelName << std::endl;
}

void Server::_handleQUIT(Client *client, const std::string &args) {
	(void)args;

	const std::set<std::string> &chans = client->getChannels();
	for (std::set<std::string>::const_iterator it = chans.begin(); it != chans.end(); ++it) {
		Channel *channel = _findChannel(*it);
		if (channel) {
			std::string quitMsg = ":" + client->getNickname()
			                + "!" + client->getUser()
			                + "@server QUIT :" + args;
			channel->broadcast(quitMsg);
			channel->removeMember(client->getFd());
			if (channel->getMemberCount() == 0) {
				delete channel;
				_channels.erase(*it);
			}
		}
	}

	_removeClient(client->getFd());
}
