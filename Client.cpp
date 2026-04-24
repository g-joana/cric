#include "Client.hpp"

Client::Client(int fd) : _fd(fd), _nickname(""), _user(""), _parser(), _isRegistered(false) {
}

Client::~Client() {
}

int Client::getFd() const {
    return _fd;
}

std::string Client::getNickname() const {
    return _nickname;
}

void Client::setNickname(const std::string &nick) {
    _nickname = nick;
}

std::string Client::getUser() const {
    return _user;
}

void Client::setUser(const std::string &username) {
    _user = username;
}

bool Client::getIsRegistered() const {
    return _isRegistered;
}

void Client::setIsRegistered(bool state) {
    _isRegistered = state;
}

// Parser delegation methods
void Client::appendToBuffer(const std::string &data) {
    _parser.appendData(data);
}

bool Client::hasCompleteCommand() const {
    return _parser.hasCompleteCommand();
}

std::string Client::extractCommand() {
    return _parser.extractCommand();
}

std::string Client::getBuffer() const {
    return _parser.getBuffer();
}

void Client::clearBuffer() {
    _parser.clearBuffer();
}