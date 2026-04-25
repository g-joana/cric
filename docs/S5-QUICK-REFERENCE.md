# S5 Quick Reference Guide

## Commands Summary

### KICK - Remove User
```
Usage:  /kick #channel username
Auth:   Operator only
Errors: 482 (not op), 403 (no channel), 442 (not on channel)
```

### INVITE - Add User
```
Usage:  /invite username #channel
Auth:   Operator (if +i), otherwise any member
Errors: 482 (not op on +i), 403 (no channel), 443 (already on channel)
```

### TOPIC - View/Change Topic
```
View:   /topic #channel
Set:    /topic #channel :new topic here
Auth:   Operator (if +t), otherwise any member to view
Errors: 482 (not op on +t), 403 (no channel), 442 (not on channel)
```

### MODE - Manage Channel
```
Syntax: /mode #channel [+|-]modes [param]

+i  - Invite-only (no params)      /mode #ch +i
+t  - Topic restricted (no params)  /mode #ch +t
+k  - Password (requires key)       /mode #ch +k password
+o  - Operator (requires nick)      /mode #ch +o bob
+l  - User limit (requires number)  /mode #ch +l 50

Remove modes with minus:            /mode #ch -i    /mode #ch -k
```

---

## Permission Model

| Command | Regular User | Operator |
|---------|-------------|----------|
| KICK    | ❌          | ✅       |
| INVITE  | ✅ (if no +i) | ✅ (always) |
| TOPIC   | ✅ (view)   | ✅ (set even with +t) |
| MODE    | ❌          | ✅       |
| PRIVMSG | ✅          | ✅       |
| JOIN    | ✅ (if invited/no +i/no +l/key match) | ✅ (always) |

---

## Testing with IRSSI

### Start Server
```bash
./ircserv 6667 password123
```

### Connect with IRSSI
```bash
irssi -c 127.0.0.1 -p 6667 -n alice -w password123
```

### Test Scenarios

#### 1. Operator vs Regular User
```
/join #test          (alice is first → becomes operator)
/nick bob            (different terminal, bob connects)
/join #test          (bob joins as regular user)

as alice (op):
/kick #test bob      ✓ Works
/mode #test +i       ✓ Works

as bob (regular):
/kick #test alice    ✗ ERR_CHANOPRIVSNEEDED (482)
/mode #test +t       ✗ ERR_CHANOPRIVSNEEDED (482)
```

#### 2. MODE +i (Invite-only)
```
as alice (op):
/mode #vip +i        (set invite-only)

as charlie (non-member):
/join #vip           ✗ Cannot join

as alice (op):
/invite charlie #vip (adds charlie to invite list)

as charlie:
/join #vip           ✓ Now works

as david (not invited):
/join #vip           ✗ Cannot join
```

#### 3. MODE +t (Topic restricted)
```
as alice (op):
/mode #general +t    (restrict topic to ops)

as bob (regular):
/topic #general      ✓ Shows current topic
/topic #general :new ✗ ERR_CHANOPRIVSNEEDED (482)

as alice (op):
/topic #general :new ✓ Works
```

#### 4. MODE +k (Password)
```
as alice (op):
/mode #private +k secretpass

as charlie (not on channel):
/join #private       ✗ Need password
/join #private secretpass ✗ (not yet implemented in JOIN)
```

#### 5. MODE +l (User limit)
```
as alice (op):
/mode #small +l 2    (max 2 users)

(alice + bob = 2, channel full)

as charlie:
/join #small         ✗ Cannot join (+l)

as alice:
/kick #small bob

as charlie:
/join #small         ✓ Now works
```

#### 6. MODE +o (Operator privilege)
```
as alice (op):
/mode #general +o bob   (make bob operator)

as bob (now op):
/kick #general charlie  ✓ Now works

/mode #general -o bob   (remove bob's op status)

as bob (regular again):
/kick #general david    ✗ ERR_CHANOPRIVSNEEDED (482)
```

---

## Implementation Details

### Files Modified
- `Channel.hpp`: +5 mode members, +14 methods
- `Channel.cpp`: +120 lines of mode management
- `Server.hpp`: +4 handler declarations
- `Server.cpp`: +500 lines total (handlers + dispatcher)
- `docs/sprints_knowledge/S5-OPERATORS-DESIGN.md`: Complete documentation

### Compilation
```bash
make              # Builds ircserv binary
make clean        # Removes object files
make re           # Rebuilds from scratch
```

### Running Tests
```bash
# Terminal 1: Start server
./ircserv 6667 mypassword

# Terminal 2 & 3: Connect IRSSI clients
irssi -c 127.0.0.1 -p 6667 -n alice -w mypassword
irssi -c 127.0.0.1 -p 6667 -n bob -w mypassword

# Use /join, /mode, /kick, /invite, /topic commands
```

---

## IRC Error Codes Used

| Code | Numeric | Description |
|------|---------|-------------|
| 401  | Numeric | No such nick/channel |
| 403  | Numeric | No such channel |
| 441  | Numeric | They aren't on that channel |
| 442  | Numeric | You're not on that channel |
| 443  | Numeric | is already on channel |
| 451  | Numeric | You have not registered |
| 461  | Numeric | Not enough parameters |
| 482  | Numeric | **You're not channel operator** |
| 501  | Numeric | Invalid MODE parameter |

---

## State After S5

The server now supports:
- ✅ Complete authentication (PASS/NICK/USER)
- ✅ Multi-user channels with message broadcasting
- ✅ Operator-based permission system
- ✅ 5 channel modes with independent management
- ✅ KICK, INVITE, TOPIC, MODE commands
- ✅ Proper IRC error handling

Next sprint can build on this foundation to add:
- Server-to-server communication
- Additional commands (PART with reason, NOTICE, etc.)
- Ban/exempt lists
- More sophisticated user modes
