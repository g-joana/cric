#ifndef SERVER_HPP
# define SERVER_HPP

# include <string>
# include <vector>
# include <poll.h>
# include <map>
# include "Client.hpp"

class Client;
class Server {
public:
	Server(int port, const std::string &password);
	~Server();

	void run();

private:
	int			_port;
	std::string	_password;
	int			_fd;

	std::vector<struct pollfd>	_pollfds;
	std::map<int, Client *> _clients;
	
	// Pending connections (to avoid modifying _pollfds during poll loop)
	std::vector<int> _pendingConnections;
	
	void _acceptClient();
	void _processPendingConnections();

	// Command handlers
	void _handlePASS(Client *client, const std::string &params);
	void _handleNICK(Client *client, const std::string &params);
	void _handleUSER(Client *client, const std::string &params);
	void _sendWelcome(Client *client);
	bool _isNickDuplicate(const std::string &nick) const;
	void _removeClient(int fd);
	void _processCommand(Client *client, const std::string &command);

	// Private so we can't create a server without a port and password
	Server();
	Server(const Server &other);
	// Private so we can't copy a server/ it would mean to duplicate the fd, and cause 2 objs to use the same fd
	Server &operator=(const Server &other);
};

#endif
