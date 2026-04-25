#ifndef CHANNEL_HPP
# define CHANNEL_HPP

# include <string>
# include <map>
# include <set>
# include "Client.hpp"

class Client;

class Channel {
private:
    std::string _name;
    std::string _topic;
    std::map<int, Client*> _members;
    std::set<int> _operators;

    Channel();
    Channel(const Channel &other);
    Channel &operator=(const Channel &other);

public:
    Channel(const std::string &name);
    ~Channel();

    std::string getName() const;
    std::string getTopic() const;
    void setTopic(const std::string &topic);

    void addMember(Client *client);
    void removeMember(int fd);
    bool isMember(int fd) const;
    bool isOperator(int fd) const;
    void addOperator(int fd);
    void removeOperator(int fd);

    void broadcast(const std::string &message, int excludeFd = -1);
    size_t getMemberCount() const;
};

#endif