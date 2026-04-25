#include "Channel.hpp"
#include <iostream>

Channel::Channel(const std::string &name) 
    : _name(name), _topic(""), _inviteOnly(false), 
      _topicRestricted(false), _key(""), _userLimit(0) {
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
    _invited.erase(fd);
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

// Invitation system
void Channel::addInvite(int fd) {
    _invited.insert(fd);
}

void Channel::removeInvite(int fd) {
    _invited.erase(fd);
}

bool Channel::isInvited(int fd) const {
    return _invited.find(fd) != _invited.end();
}

// Mode management
bool Channel::isInviteOnly() const {
    return _inviteOnly;
}

void Channel::setInviteOnly(bool value) {
    _inviteOnly = value;
}

bool Channel::isTopicRestricted() const {
    return _topicRestricted;
}

void Channel::setTopicRestricted(bool value) {
    _topicRestricted = value;
}

std::string Channel::getKey() const {
    return _key;
}

void Channel::setKey(const std::string &key) {
    _key = key;
}

bool Channel::hasKey() const {
    return !_key.empty();
}

int Channel::getUserLimit() const {
    return _userLimit;
}

void Channel::setUserLimit(int limit) {
    _userLimit = limit;
}

bool Channel::isAtUserLimit() const {
    if (_userLimit == 0)
        return false;
    return static_cast<int>(_members.size()) >= _userLimit;
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

const std::map<int, Client*> &Channel::getMembers() const {
    return _members;
}