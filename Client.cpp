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

std::string Client::setNickname(const std::string &nick) const {
    /*todo*/
    return nick;
}

void Client::appendToBuffer(const std::string &message) {
    _buffer += message;
}

std::string Client::getBuffer() const {
    return _buffer;
}

void Client::clearBuffer() {
    _buffer.clear();
}