# S4 - Channel System (JOIN/PART/QUIT/Broadcast)

## Sprint Overview

Implemented a complete channel system for ft_irc with JOIN, PART, QUIT commands and message broadcast functionality.

**Status**: COMPLETED  
**Timeline**: ~2 hours  
**Blocker**: NO  

---

## What S3 Provided

S3 delivered PRIVMSG user→user functionality:
- `_findClientByNick()` - client lookup
- `_handlePRIVMSG()` - user direct messages
- Authentication system (S2)

S4 built on this to implement channels where messages broadcast to all members.

---

## Architecture

### Classes Added

```
Channel (NEW)
├── _name: string
├── _topic: string  
├── _members: map<int, Client*>
├── _operators: set<int>
└── Methods: add/remove member, broadcast, isMember/isOperator

Server
├── _channels: map<string, Channel*> (NEW)
├── _handleJOIN()
├── _handlePART()
├── _handleQUIT()
└── _findChannel()

Client
├── _channels: set<string> (NEW)
├── addChannel(), removeChannel(), isInChannel()
└── getChannels()
```

### Data Flow

```
Client sends "JOIN #channel"
    ↓
Server._handleJOIN()
    ↓
Create Channel if not exists
    ↓
Add member + make creator operator
    ↓
Broadcast JOIN to existing members (exclude sender)
    ↓
Send RPL_331/332 (topic), RPL_353/366 (names)
```

---

## Implementation Details

### Channel Class

```cpp
class Channel {
    // Members stored by fd for O(1) lookup
    std::map<int, Client*> _members;
    // Operators subset
    std::set<int> _operators;
    
    // Broadcast sends to all except excludeFd
    void broadcast(const std::string &msg, int excludeFd = -1);
};
```

### JOIN Command

**Flow**:
1. Validate registered client
2. Parse channel name (must start with `#`)
3. Create channel if not exists
4. Add member, add creator as operator
5. Broadcast JOIN to existing members (sender excluded)
6. Send topic (RPL_331/332), names list (RPL_353/366)

**RFC Responses**:
- RPL_331 (331): `:server 331 <nick> <channel> :No topic is set`
- RPL_332 (332): `:server 332 <nick> <channel> :<topic>`
- RPL_353 (353): `:server 353 <nick> = <channel> :<names>`
- RPL_366 (366): `:server 366 <nick> <channel> :End of /NAMES list`

### PART Command

**Flow**:
1. Validate registered, verify member of channel
2. Remove from channel members/operators
3. Broadcast PART to remaining members
4. Send PART confirmation to sender
5. Delete channel if empty

### QUIT Command

**Flow**:
1. Iterate all channels client is in
2. Broadcast QUIT to each channel
3. Remove from all channels
4. Clean up empty channels
5. Call `_removeClient()` for full cleanup

**Note**: QUIT is triggered automatically when client disconnects (bytesRead <= 0) in `Server::run()`

### Broadcast Logic

```cpp
void Channel::broadcast(const std::string &msg, int excludeFd) {
    for (map<int, Client*>::iterator it = _members.begin();
         it != _members.end(); ++it) {
        if (it->first != excludeFd)
            it->second->sendMessage(msg);
    }
}
```

The `excludeFd` parameter prevents sending back to the sender (important for JOIN).

### Channel PRIVMSG

Extended from S3 user→user PRIVMSG:

```cpp
if (target[0] == '#') {
    Channel *channel = _findChannel(target);
    if (!channel) return ERR_NOSUCHCHANNEL;
    if (!channel->isMember(client->getFd())) return ERR_NOTONCHANNEL;
    channel->broadcast(msg, client->getFd());
    return;
}
// Else: user→user PRIVMSG (S3 logic)
```

---

## Edge Cases Handled

| Case | Handling |
|------|----------|
| JOIN non-existent channel | Create new channel automatically |
| JOIN without `#` prefix | Return 403 No such channel |
| PART from non-member channel | Return 442 Not on channel |
| PART when last member | Delete channel, free memory |
| QUIT notification | Broadcast to all channels client was in |
| Channel empty after removal | Delete channel object |

---

## Memory Management

- Channels allocated with `new` in `_handleJOIN`
- Channels deallocated when `getMemberCount() == 0` in PART/QUIT
- Server destructor cleans up all remaining channels
- Client pointer stored by fd (stable during connection)

---

## Testing

```bash
# Test S4-acceptance.sh
./test/S4-acceptance.sh

# Manual test
./ircserv 6667 password &
# Client 1: JOIN #test
# Client 2: JOIN #test
# Client 2: PRIVMSG #test :Hello
# Client 1 should receive bob's message
```

---

## Dependencies

- S2: Authentication (required for JOIN/PART/PRIVMSG)
- S3: PRIVMSG routing (extended for channels)

---

## Next Steps (S5)

S5 will add operator commands:
- KICK: Remove user from channel
- INVITE: Invite user to channel (+i mode)
- TOPIC: View/change topic (+t mode)
- MODE: Channel settings (+i, +t, +k, +o, +l)

The `Channel` class already has `_operators` set and `isOperator()` method ready for S5.