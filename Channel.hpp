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
    std::set<int> _invited;
    
    // Channel modes
    bool _inviteOnly;          // +i
    bool _topicRestricted;     // +t (only ops can set topic)
    std::string _key;          // +k (password)
    int _userLimit;            // +l (max users, 0 = unlimited)

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

    // Invitation system
    void addInvite(int fd);
    void removeInvite(int fd);
    bool isInvited(int fd) const;

    // Mode management
    bool isInviteOnly() const;
    void setInviteOnly(bool value);
    
    bool isTopicRestricted() const;
    void setTopicRestricted(bool value);
    
    std::string getKey() const;
    void setKey(const std::string &key);
    bool hasKey() const;
    
    int getUserLimit() const;
    void setUserLimit(int limit);
    bool isAtUserLimit() const;

    void broadcast(const std::string &message, int excludeFd = -1);
    size_t getMemberCount() const;
    const std::map<int, Client*> &getMembers() const;
};

#endif