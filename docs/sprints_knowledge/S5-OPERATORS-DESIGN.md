# S5 - Operators & Moderation System

## 🎯 Sprint Overview

Sprint S5 implements a complete **channel operator system** with permission-based command execution. This sprint introduces:

- **4 core operator commands**: KICK, INVITE, TOPIC, MODE
- **5 channel modes**: `+i` (invite-only), `+t` (topic restricted), `+k` (password), `+o` (operator privilege), `+l` (user limit)
- **Permission system**: Only channel operators can execute restricted commands
- **Error handling**: Proper IRC error codes (482 ERR_CHANOPRIVSNEEDED, 403, 442, etc.)

**Status**: ✅ COMPLETED  
**Timeline**: 3 hours  
**Blocker**: NO (Non-critical, enhances functionality)

---

## 🏗️ Architecture & Design

### Permission Model

```
Channel Operator Privileges:
├── KICK: Can remove users from channel
├── INVITE: Can invite users (especially when +i mode is set)
├── TOPIC: Can change topic (when +t mode restricts non-ops)
└── MODE: Can set/modify channel modes (+i, +t, +k, +o, +l)

Regular User:
├── Can view READ operations (TOPIC without params shows current topic)
└── Cannot execute WRITE operations without operator privilege
```

### Data Structure Extensions

#### Channel.hpp - New Members
```cpp
// Moderation
std::set<int> _invited;         // Users invited to invite-only channels

// Channel modes
bool _inviteOnly;               // Mode +i
bool _topicRestricted;          // Mode +t
std::string _key;               // Mode +k (password)
int _userLimit;                 // Mode +l (max users, 0 = unlimited)
```

#### New Channel Methods
```cpp
// Invitation system
void addInvite(int fd);         // Add user to invite list
void removeInvite(int fd);      // Remove from invite list
bool isInvited(int fd) const;   // Check if user is invited

// Mode getters/setters
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
```

---

## 📋 Commands Implemented

### KICK - Remove User from Channel

**RFC 1459**: Forcibly remove a user from a channel

**Syntax**: `KICK <channel> <user>\r\n`

**Behavior**:
1. Sender must be registered
2. Sender must be in target channel
3. Sender must be channel operator
4. Target user must exist and be in channel
5. Remove target from channel, broadcast to all members

**Error Codes**:
- `451`: User not registered
- `403`: No such channel
- `442`: Sender not on channel
- `482`: Sender not channel operator
- `401`: Target user not found
- `441`: Target not on that channel

**Example Flow**:
```
Client (op) → KICK #general spammer
Server → :alice!alice@server KICK #general spammer
         (broadcasted to all channel members)
         spammer removed from #general
```

---

### INVITE - Invite User to Channel

**RFC 1459**: Invite a user to join a channel

**Syntax**: `INVITE <user> <channel>\r\n`

**Behavior**:
1. Sender must be registered
2. Sender must be in target channel
3. If channel has +i mode, sender must be operator
4. Target user must exist and not already be on channel
5. Add target to invite list
6. Send INVITE message to target, 341 reply to sender

**Error Codes**:
- `451`: User not registered
- `403`: No such channel
- `442`: Sender not on channel
- `482`: Sender not operator (only if +i mode set)
- `401`: Target user not found
- `443`: Target already on that channel

**Example Flow**:
```
Client (op) → INVITE friend #vip
Server → :alice!alice@server INVITE friend :#vip
         (sent to 'friend')
         :server 341 alice friend #vip
         (sent to alice - confirmation)
```

---

### TOPIC - View/Change Channel Topic

**RFC 1459**: Get or set the channel topic

**Syntax**:
- View: `TOPIC <channel>\r\n`
- Set: `TOPIC <channel> :<new topic>\r\n`

**Behavior (View)**:
1. Sender must be registered and on channel
2. If topic exists: send 332 reply (topic text)
3. If no topic: send 331 reply (no topic set)

**Behavior (Set)**:
1. Sender must be registered and on channel
2. If channel has +t mode, sender must be operator
3. Set new topic
4. Broadcast TOPIC message to all channel members

**Error Codes**:
- `451`: User not registered
- `403`: No such channel
- `442`: Sender not on channel
- `482`: Sender not operator (only if +t mode set)

**Example Flow**:
```
VIEW:
Client → TOPIC #general
Server → :server 332 alice #general :Welcome to General Chat

SET:
Client (op) → TOPIC #general :New rules: be respectful
Server → :alice!alice@server TOPIC #general :New rules: be respectful
         (broadcasted to all channel members)
```

---

### MODE - Manage Channel Modes

**RFC 1459-inspired**: Change channel modes

**Syntax**: `MODE <channel> [+|-]<modes> [parameters]\r\n`

**Modes**:

#### `+i/-i` - Invite Only
- **Meaning**: Channel requires invitation to join
- **Parameters**: None
- **Effect**: New users must be invited (see INVITE command)

#### `+t/-t` - Topic Restricted
- **Meaning**: Only operators can change topic
- **Parameters**: None
- **Effect**: TOPIC command restricted to operators

#### `+k/-k` - Channel Key (Password)
- **Meaning**: Users must supply password to join
- **Parameters**: `<key>` when setting, none when removing
- **Effect**: JOIN command requires matching key

#### `+o/-o` - Channel Operator
- **Meaning**: Grant or revoke operator privilege
- **Parameters**: `<nickname>`
- **Effect**: User gains/loses operator permissions

#### `+l/-l` - User Limit
- **Meaning**: Maximum number of users in channel
- **Parameters**: `<number>` when setting, none when removing
- **Effect**: Channel refuses JOIN if at limit

**Error Codes**:
- `451`: User not registered
- `403`: No such channel
- `442`: Sender not on channel
- `482`: Sender not operator
- `461`: Not enough parameters
- `501`: Invalid parameter (e.g., non-numeric for +l)

**Example Flows**:

```
SET INVITE-ONLY:
Client (op) → MODE #vip +i
Server → :alice!alice@server MODE #vip +i
         (broadcasted to all)

SET PASSWORD:
Client (op) → MODE #private +k secretpass
Server → :alice!alice@server MODE #private +k secretpass

MAKE OPERATOR:
Client (op) → MODE #general +o bob
Server → :alice!alice@server MODE #general +o bob

SET USER LIMIT:
Client (op) → MODE #lobby +l 50
Server → :alice!alice@server MODE #lobby +l 50

REMOVE MODE:
Client (op) → MODE #vip -i
Server → :alice!alice@server MODE #vip -i
```

---

## 🔒 Permission Checks

### Operator-Only Operations

All KICK, INVITE (with +i), TOPIC (with +t), and all MODE changes require:

```
if (!channel->isOperator(client->getFd())) {
    // Send ERROR_CHANOPRIVSNEEDED (482)
}
```

### User Assignment on JOIN

Currently, **first user to join a channel becomes operator**:

```cpp
void Server::_handleJOIN(Client *client, const std::string &args) {
    // ... create channel or get existing ...
    channel->addMember(client);
    channel->addOperator(client->getFd());  // ← First user is op
    // ...
}
```

---

## 📤 IRC Messages Reference

### User to Server

```
:client KICK #channel target
:client INVITE user #channel
:client TOPIC #channel :new topic
:client MODE #channel +i
:client MODE #channel +o bob
:client MODE #channel +k password
:client MODE #channel +l 100
```

### Server to User (Responses & Broadcasts)

```
401: No such nick/channel
403: No such channel
441: They aren't on that channel
442: You're not on that channel
443: is already on channel
451: You have not registered
461: Not enough parameters
482: You're not channel operator
501: Invalid MODE parameter

341: <client> <user> <channel>           (INVITE confirmation)
331: <client> <channel> :No topic is set
332: <client> <channel> :<topic>         (TOPIC view)

:op!user@server KICK #channel target    (broadcast)
:op!user@server INVITE target :#channel (to target)
:op!user@server TOPIC #channel :topic   (broadcast)
:op!user@server MODE #channel +i        (broadcast)
```

---

## 🧪 Validation Examples

### Test Scenario 1: KICK Without Operator Privilege

```
alice (regular) → /kick #general bob
Server → :server 482 alice #general :You're not channel operator
Result → alice cannot kick bob ✓
```

### Test Scenario 2: MODE +i Then INVITE Required

```
alice (op) → /mode #vip +i
bob (not invited) → /join #vip
Server → :server 473 bob #vip :Cannot join channel (+i)
alice → /invite bob #vip
bob → /join #vip
Server → bob joins successfully ✓
```

### Test Scenario 3: MODE +t Restricts TOPIC

```
alice (op) → /mode #general +t
bob (regular) → /topic #general :New topic
Server → :server 482 bob #general :You're not channel operator
alice → /topic #general :New topic
Server → (accepts, broadcasts) ✓
```

### Test Scenario 4: MODE +l User Limit

```
alice (op) → /mode #small +l 2
(channel has 2 members)
charlie → /join #small
Server → :server 471 charlie #small :Cannot join channel (+l)
alice → /kick #small bob
charlie → /join #small
Server → charlie joins successfully ✓
```

---

## 🔧 Impact on JOIN Command

S5 modifies JOIN behavior:

```cpp
// If channel is invite-only:
if (channel->isInviteOnly()) {
    if (!channel->isInvited(clientFd)) {
        // Send error, refuse join
    }
    channel->removeInvite(clientFd);  // One-time use
}

// If channel is at user limit:
if (channel->isAtUserLimit()) {
    // Send error 471, refuse join
}

// If channel has password:
if (channel->hasKey() && providedKey != channel->getKey()) {
    // Send error 475, refuse join
}
```

---

## 📊 State Machine After S5

```
REGISTERED Client
    ↓
    │ /JOIN #channel (first user)
    ├─→ Becomes Operator
    │
    ├─→ /KICK, /INVITE (ops only)
    ├─→ /TOPIC (with +t restriction)
    ├─→ /MODE (manage all aspects)
    │
    ├─→ /PRIVMSG #channel (to all members)
    ├─→ /PART (leave channel)
    └─→ /QUIT (disconnect)
```

---

## 📝 Code Quality Notes

- **C++98 Compliance**: All new code uses C++98 standard features
- **Non-blocking I/O**: Handlers are non-blocking, integrated with poll()
- **Error Handling**: Comprehensive IRC error codes
- **Memory Safety**: No new allocations in handlers, proper cleanup
- **Clean Code**: Clear separation of concerns between handlers

---

## ✅ Checklist

- [x] Channel class extended with mode tracking
- [x] KICK handler with operator permission check
- [x] INVITE handler with mode-aware behavior
- [x] TOPIC handler with +t mode restriction
- [x] MODE handler supporting +i, +t, +k, +o, +l
- [x] Proper error codes (482 ERR_CHANOPRIVSNEEDED, etc.)
- [x] Broadcast messages to all channel members
- [x] Code compiles with -Wall -Wextra -Werror -std=c++98
- [x] Ready for S3+ integration
