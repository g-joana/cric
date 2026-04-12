#include <iostream>
#include <cstdlib>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char **argv) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <port> <password>" << std::endl;
        return 1;
    }

    int port = std::atoi(argv[1]);
    std::string password = argv[2];
    (void)password;

    // setup sockets
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        std::cerr << "Failed to create socket" << std::endl;
        return 1;
    }
    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        std::cerr << "Failed to bind socket" << std::endl;
        return 1;
    }
    if (listen(fd, 10) == -1) {
        std::cerr << "Failed to listen on socket" << std::endl;
        return 1;
    }
    if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1) {
        std::cerr << "Failed to set non-blocking mode" << std::endl;
        return 1;
    }
    std::cout << "Server is runnning! Socket created and listening on port " << port << " :)" << std::endl;

    // poll/ epoll loop to accept connections

    // read/write data from/to clients

    // business logic (commands, private messages, channels, group messages, operators)

    return 0;
}