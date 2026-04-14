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
	void _acceptClient();

	// Private so we can't create a server without a port and password
	Server();
	Server(const Server &other);
	// Private so we can't copy a server/ it would mean to duplicate the fd, and cause 2 objs to use the same fd
	Server &operator=(const Server &other);
};

#endif
