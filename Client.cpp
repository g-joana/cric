#include "Client.hpp"

Client::Client(int fd) : _fd(fd), _nickname(""), _user(""), _realname(""),
                          _parser(), _isRegistered(false), _isAuthenticated(false),
                          _hasNick(false), _hasUser(false), _state(INIT) {
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

std::string Client::getRealname() const {
    return _realname;
}

void Client::setRealname(const std::string &realname) {
    _realname = realname;
}

bool Client::getIsRegistered() const {
    return _isRegistered;
}

void Client::setIsRegistered(bool state) {
    _isRegistered = state;
}

bool Client::getIsAuthenticated() const {
    return _isAuthenticated;
}

void Client::setIsAuthenticated(bool state) {
    _isAuthenticated = state;
}

bool Client::getHasNick() const {
    return _hasNick;
}

void Client::setHasNick(bool state) {
    _hasNick = state;
}

bool Client::getHasUser() const {
    return _hasUser;
}

void Client::setHasUser(bool state) {
    _hasUser = state;
}

ClientState Client::getState() const {
    return _state;
}

void Client::setState(ClientState state) {
    _state = state;
}

bool Client::canTransitionToREGISTERED() const {
    return _hasNick && _hasUser && _isAuthenticated;
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