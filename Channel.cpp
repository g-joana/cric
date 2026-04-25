#include "Channel.hpp"
#include <iostream>

Channel::Channel(const std::string &name) : _name(name), _topic("") {
}

Channel::~Channel() {
}

std::string Channel::getName() const {
    return _name;
}

std::string Channel::getTopic() const {
    return _topic;
}

void Channel::setTopic(const std::string &topic) {
    _topic = topic;
}

void Channel::addMember(Client *client) {
    if (client) {
        _members[client->getFd()] = client;
    }
}

void Channel::removeMember(int fd) {
    _members.erase(fd);
    _operators.erase(fd);
}

bool Channel::isMember(int fd) const {
    return _members.find(fd) != _members.end();
}

bool Channel::isOperator(int fd) const {
    return _operators.find(fd) != _operators.end();
}

void Channel::addOperator(int fd) {
    if (isMember(fd)) {
        _operators.insert(fd);
    }
}

void Channel::removeOperator(int fd) {
    _operators.erase(fd);
}

void Channel::broadcast(const std::string &message, int excludeFd) {
    for (std::map<int, Client*>::iterator it = _members.begin();
         it != _members.end(); ++it) {
        if (it->first != excludeFd) {
            it->second->sendMessage(message);
        }
    }
}

size_t Channel::getMemberCount() const {
    return _members.size();
}