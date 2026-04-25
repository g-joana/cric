#ifndef CLIENT_HPP
# define CLIENT_HPP

# include <string>
# include <vector>
# include <set>
# include <poll.h>
# include <iostream>
# include "CommandParser.hpp"

enum ClientState {
    INIT,        // Connected, awaiting PASS
    AUTH,        // PASS OK, awaiting NICK and USER
    ID,          // NICK OK, awaiting USER
    REGISTERED   // NICK+USER OK, fully authenticated
};

class Client{
    private: 
        int         _fd;
        std::string _nickname;
        std::string _user;
        std::string _realname;
        CommandParser _parser;
        bool _isRegistered;
        bool _isAuthenticated;
        bool _hasNick;
        bool _hasUser;
        ClientState _state;
        std::set<std::string> _channels;

        Client(); // nao criar client sem fd;

    public:
        Client(int fd);
        ~Client();

        int getFd() const;
        
        std::string getNickname() const;
        void setNickname(const std::string &nick); 

        std::string getUser() const;
        void setUser(const std::string &username);

        std::string getRealname() const;
        void setRealname(const std::string &realname);

        bool getIsRegistered() const;
        void setIsRegistered(bool state);

        bool getIsAuthenticated() const;
        void setIsAuthenticated(bool state);

        bool getHasNick() const;
        void setHasNick(bool state);

        bool getHasUser() const;
        void setHasUser(bool state);

        ClientState getState() const;
        void setState(ClientState state);

        bool canTransitionToREGISTERED() const;

        // Parser integration
        void appendToBuffer(const std::string &data);
        bool hasCompleteCommand() const;
        std::string extractCommand();
        std::string getBuffer() const; 
        void clearBuffer(); 

        void sendMessage(const std::string &msg);

        void addChannel(const std::string &channel);
        void removeChannel(const std::string &channel);
        bool isInChannel(const std::string &channel) const;
        const std::set<std::string> &getChannels() const;
};

#endif 