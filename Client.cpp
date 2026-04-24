#include "Client.hpp"

Client::Client(int fd) : _fd(fd), _nickname(""), _user(""), _buffer(""), isRegistered("false") {
}

Client::~Client() {
}

int Client::getFd() const {
    return _fd;
}

std::string Client::getNickname() const {
    return _nickname;
}

void Client::setNickname(const std::string &nick) const {
    _nickname = nick;
}

std::string Client::getUser() const {
    return _user;
}

void Client::setUser(const std::string &username) const {
    _user = username;
}

bool Client::getIsRegistered() const {
    return _isRegistered;
}

void Client::setIsRegistered(bool state) {
    return state;
}

std::string Client::getBuffer() const {
    return _buffer;
}

void Client::appendToBuffer(const std::string &message) {
    _buffer += message;
}

void Client::clearBuffer() {
    std::cout << "clear buffer" << std::endl;
    _buffer.clear();
}