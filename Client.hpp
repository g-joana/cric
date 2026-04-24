#ifndef CLIENT_HPP
# define CLIENT_HPP

# include <string>
# include <vector>
# include <poll.h>

class Client{
    private: 
        int         _fd;
        std::string _nickname;
        std::string _user;
        std::string _buffer;
        bool _isRegistered;

        Client(); // nao criar client sem fd;

    public:
        Client(int fd);
        ~Client();

        int getFd() const;
        std::string getNickname() const;
        std::string setNickname(const std::string &nick) const; 

        void appendToBuffer(const std::string &message); //montar mensagem parcelada no buffer
        std::string getBuffer() const; 
        void clearBuffer(); 
};


#endif 