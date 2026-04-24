#include "Client.hpp"

Client::Client(int fd) : _fd(fd), _nickname(""), _user(""), _buffer(""), _isRegistered(false) {
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