# S2 - Authentication System (PASS/NICK/USER)

## 🎯 Sprint Overview

Sprint S2 completed the implementation of a complete IRC authentication system with:
- **PASS** command for password validation
- **NICK** command for nickname management with duplicate detection
- **USER** command for user identity
- **State Machine** controlling client lifecycle (INIT → AUTH → ID → REGISTERED)
- **RPL_WELCOME (001)** message upon successful registration

**Status**: ✅ COMPLETED  
**Timeline**: 2.5 hours  
**Blocker**: NO (Critical foundation for S3+)

---

## 🔄 What S1 Provided

S1 delivered the foundational parser and bug fixes needed for S2:
- ✅ CommandParser aggregating fragmented packets (`\r\n` delimited)
- ✅ Client buffer handling with `appendToBuffer()`, `hasCompleteCommand()`, `extractCommand()`
- ✅ Server loop stable with `poll()` and pending connection handling
- ✅ No memory leaks or segmentation faults

S2 built directly on this foundation to implement stateful authentication.

---

## 🏗️ Architecture & Design

### Client State Machine

Four distinct states controlling the authentication flow:

```
INIT
  │
  ├─ PASS (password validation)
  │   │
  │   ├─ CORRECT → AUTH state
  │   └─ INCORRECT → Error 464 + Disconnect
  │
  └─[AUTH]
      │
      ├─ NICK (set nickname, no duplicate allowed)
      │   │
      │   ├─ VALID & UNIQUE → ID state, response ":server NICK <nick>"
      │   └─ DUPLICATE → Error 433, stay in AUTH
      │
      └─[ID]
          │
          ├─ USER (set username + realname)
          │   │
          │   ├─ VALID & have NICK → REGISTERED state
          │   │   └─ Send RPL_WELCOME (001)
          │   └─ INVALID → Error 461, stay in ID
          │
          └─[REGISTERED] ← Ready for S3+ commands (PRIVMSG, JOIN, etc.)
```

### State Transitions in Code

```cpp
// In Client.hpp
enum ClientState {
    INIT,        // Connected, awaiting PASS
    AUTH,        // PASS OK, awaiting NICK and USER
    ID,          // NICK OK, awaiting USER
    REGISTERED   // NICK+USER OK, fully authenticated
};

// Helper to check readiness
bool Client::canTransitionToREGISTERED() const {
    return _hasNick && _hasUser && _isAuthenticated;
}
```

---

## 📋 Commands Implemented

### PASS: Password Validation

**RFC 1459**: Authenticate with server password

**Syntax**: `PASS <password>\r\n`

**Behavior**:
- Compares password with `Server::_password`
- Correct: Mark authenticated (`_isAuthenticated = true`), state → AUTH
- Incorrect: Send error 464, disconnect immediately

**Example Flow**:
```
Client → PASS secret123
Server: (validates against stored password)
        → If correct: (no response, state = AUTH)
        → If wrong: :server 464 * :Incorrect password\r\n
                    (disconnect)
```

### NICK: Nickname Management

**RFC 1459**: Define or change nickname

**Syntax**: `NICK <nickname>\r\n`

**Behavior**:
- Validate: not empty, no spaces, not duplicate
- Valid & unique: Set nickname, state → ID (or REGISTERED if USER also set)
- Duplicate: Error 433, client stays in current state

**Example Flow**:
```
Client → NICK alice
Server: (checks if "alice" already used)
        → If unique: :server NICK alice\r\n (state = ID)
        → If duplicate: :server 433 * alice :Nickname is already in use\r\n
```

### USER: User Identity

**RFC 1459**: Set username and real name

**Syntax**: `USER <username> <mode> <unused> :<realname>\r\n`

**Behavior**:
- Parse: Extract username (before mode) and realname (after `:`)
- Valid: Save username and realname
- If also have NICK: State → REGISTERED, send RPL_WELCOME (001)

**Example Flow**:
```
Client → USER alice 0 * :Alice Smith
Server: (parses: username=alice, realname="Alice Smith")
        → Save both fields
        → If already have NICK: send 001 and state = REGISTERED
        → Else: stay in ID state
```

### RPL_WELCOME (001)

**RFC 1459**: Welcome message after successful registration

**Syntax**: `:server 001 <nick> :Welcome to <servername>\r\n`

**Behavior**:
- Sent only when state = REGISTERED (NICK + USER + PASS all OK)
- Sent once per client after registration
- Notifies client that they're ready for normal IRC commands

**Example**:
```
:server 001 alice :Welcome to ft_irc\r\n
```

---

## 🧪 Testing Strategy

### Test Structure (S2-T1 through S2-T6)

Each test script validates specific functionality:

#### **S2-T1: PASS Validation** (test/S2-T1-pass-validation.sh)
- ✅ C1: Correct password accepted
- ✅ C2: Wrong password rejected with error 464
- ✅ C3: Fragmented commands handled (aggregation)
- ✅ C4: Empty password handled gracefully

#### **S2-T2: NICK Validation** (test/S2-T2-nick-validation.sh)
- ✅ C1: Valid nickname accepted
- ✅ C2: Duplicate nickname rejected with error 433
- ✅ C3: Empty nickname rejected with error 461
- ✅ C4: Nickname with spaces rejected

#### **S2-T3: USER Validation** (test/S2-T3-user-validation.sh)
- ✅ C1: Valid USER with username + realname accepted
- ✅ C2: USER without realname rejected with error 461
- ✅ C3: USER with empty realname handled
- ✅ C4: USER without parameters rejected

#### **S2-T4: State Machine** (test/S2-T4-state-machine.sh)
- ✅ C1: PASS transitions INIT → AUTH
- ✅ C2: NICK transitions AUTH → ID
- ✅ C3: USER transitions ID → REGISTERED (with RPL_WELCOME)
- ✅ C4: Full sequence works: INIT → AUTH → ID → REGISTERED

#### **S2-T5: RPL_WELCOME** (test/S2-T5-welcome.sh)
- ✅ C1: RPL_WELCOME code 001 present
- ✅ C2: Welcome message text ("Welcome") present
- ✅ C3: RPL_WELCOME includes correct nickname
- ✅ C4: RPL_WELCOME not sent before USER (timing correct)

#### **S2-T6: Integration Tests** (test/S2-T6-irssi-validation.sh)
- ✅ C1: Multiple clients simultaneous authentication
- ✅ C2: Wrong password rejection
- ✅ C3: Duplicate nick rejection with active connection
- ✅ C4: Full authentication sequence works
- ✅ C5: Fragmented commands handled safely
- ✅ C6: Empty input handled safely

### Acceptance Criteria (test/S2-acceptance.sh)

Master test combining all sub-tests:
1. ✅ Compilation with `-Wall -Wextra -Werror -std=c++98`
2. ✅ S1 regression (parser still works)
3. ✅ S2-T1 through S2-T6 all pass
4. ✅ Valgrind clean (zero memory leaks)

**Result**: All criteria PASSED ✅

---

## 🐛 Edge Cases Handled

### Duplicate Nicknames

**Issue**: Two clients connecting simultaneously might set the same nickname.

**Solution**: 
- Check `_isNickDuplicate()` against ALL connected clients
- If duplicate found: send 433 error, do NOT disconnect
- Client can retry with different nickname

**Test**: S2-T2-C2 and S2-T6-C3 verify this works with concurrent connections.

### Fragmented Commands

**Issue**: Commands might arrive split across packets (e.g., "PASS test" in one packet, "123\r\n" in next).

**Solution**: 
- CommandParser aggregates incomplete commands in buffer
- Only processes when complete command (ending with `\r\n`) exists
- Server calls `extractCommand()` repeatedly until buffer empty

**Test**: S2-T1-C3 and S2-T6-C5 verify fragmented handling.

### Empty or Invalid Input

**Issue**: Client sends empty lines, commands without parameters, etc.

**Solution**:
- Validate all parameters before processing
- Return appropriate error codes (461, 464, 433)
- Keep connection alive (except wrong PASS → 464 closes)

**Test**: S2-T1-C4, S2-T2-C3, S2-T3-C4, S2-T6-C6 cover edge cases.

### Multiple Concurrent Clients

**Issue**: Server must handle PASS/NICK/USER for multiple clients without mixing state.

**Solution**:
- Each Client has independent state machine (`_state`, `_isAuthenticated`, etc.)
- No global state shared between clients
- Server loops through all pollfd entries independently

**Test**: S2-T6-C1 authenticates multiple clients in parallel.

---

## 💾 Code Changes Summary

### Files Modified

#### **Client.hpp**
- Added `enum ClientState` (INIT, AUTH, ID, REGISTERED)
- Added state fields: `_state`, `_isAuthenticated`, `_hasNick`, `_hasUser`, `_realname`
- Added getters/setters for new fields
- Added `canTransitionToREGISTERED()` helper

#### **Client.cpp**
- Updated constructor to initialize state machine fields
- Implemented all new getters/setters

#### **Server.hpp**
- Added handler signatures: `_handlePASS()`, `_handleNICK()`, `_handleUSER()`
- Added helper: `_isNickDuplicate()`
- Added: `_sendWelcome()`, `_removeClient()`, `_processCommand()`

#### **Server.cpp**
- Implemented all command handlers (PASS, NICK, USER)
- Implemented `_processCommand()` dispatcher
- Integrated command dispatch in `Server::run()` loop
- Added proper error handling and response formatting

---

## 🔍 RFC 1459 Compliance

### Error Codes Used

| Code | Name | Condition |
|------|------|-----------|
| **001** | RPL_WELCOME | Client fully registered (NICK+USER+PASS OK) |
| **433** | ERR_NICKNAMEINUSE | Nick already taken by another client |
| **461** | ERR_NEEDMOREPARAMS | Command missing required parameters |
| **464** | ERR_PASSWDMISMATCH | Wrong password or PASS protocol violation |

### Protocol Details

**PASS Response**:
- On success: No response (RFC standard)
- On failure: `:server 464 * :Incorrect password\r\n` + disconnect

**NICK Response**:
- On success: `:server NICK <nick>\r\n`
- On error: `:server 433 * <nick> :Nickname is already in use\r\n`

**USER Response**:
- No immediate response (RPL_WELCOME sent later if ready)
- On error: `:server 461 * USER :Not enough parameters\r\n`

**RPL_WELCOME Format**:
- `:server 001 <nick> :Welcome to ft_irc\r\n`
- Sent when both NICK and USER are set and PASS was correct

---

## 📊 Performance & Memory

### Memory Management
- ✅ All dynamically allocated objects properly deleted
- ✅ No pointers leaked during client disconnection
- ✅ No memory leaks in error paths (wrong password, bad params)
- ✅ Valgrind verified: 0 bytes lost

### Stability
- ✅ No segmentation faults in any scenario
- ✅ Server continues running even when clients crash
- ✅ Handles 10+ concurrent clients without issues
- ✅ Survives rapid connect/disconnect cycles

---

## ✨ Dependencies for S3

S3 (PRIVMSG private messages) depends on:

1. ✅ **Client State Machine**: REGISTERED state detection
2. ✅ **Client Identity**: Nickname + Username stored correctly
3. ✅ **Command Dispatch**: Framework to add new handlers
4. ✅ **Connection Management**: _clients map and _removeClient()

All prerequisites met. S3 can proceed with PRIVMSG implementation.

---

## 🚀 Next Steps (S3)

### PRIVMSG (Private Message)

S3 will implement direct client-to-client messaging:

**Syntax**: `PRIVMSG <target> :<message>\r\n`

**Requirements**:
- Only for REGISTERED clients
- Target can be: nickname (user) or channel name
- Should route message to target client(s)
- Return error if target doesn't exist

**Dependencies on S2**:
- ✅ Client must be REGISTERED to send PRIVMSG
- ✅ Lookup target by nickname from `_clients` map
- ✅ Use established send() infrastructure

---

## 📝 Test Results Summary

```
S2-T1 (PASS)         : 4/4 ✅
S2-T2 (NICK)         : 4/4 ✅  
S2-T3 (USER)         : 4/4 ✅
S2-T4 (State Machine): 4/4 ✅
S2-T5 (Welcome)      : 4/4 ✅
S2-T6 (Integration)  : 6/6 ✅

TOTAL: 26/26 ✅ PASSED
```

**Compilation**: ✅ Clean with `-Wall -Wextra -Werror -std=c++98`  
**Regression**: ✅ S1 tests still pass  
**Memory**: ✅ Valgrind clean  
**Stability**: ✅ No crashes on any input  

---

## 📚 Key Learnings

### State Machine Complexity

Managing 4 states (INIT→AUTH→ID→REGISTERED) requires careful attention to:
- When state transitions occur (after which command)
- What happens if commands arrive out of order
- How to handle duplicate operations (e.g., NICK twice)

Solution: Make state explicit in code with enum and clear transition logic.

### NICK Duplicate Detection

Simply checking if nickname exists isn't enough - must:
- Check against ALL connected clients (not just those in a list)
- NOT disconnect client on duplicate (per RFC)
- Allow retry with different nickname

Solution: Iterate `_clients` map and return 433 without state change.

### Packet Aggregation

Commands might arrive fragmented, but server must NOT process partial commands.
Solution: Ensure CommandParser fully buffers until `\r\n` found.

---

## ✅ Acceptance Checklist

- ✅ Code compiles with strict flags
- ✅ PASS command validates password
- ✅ NICK command manages nicknames uniquely
- ✅ USER command captures identity
- ✅ State machine transitions correctly
- ✅ RPL_WELCOME (001) sent on registration
- ✅ Multiple clients work independently
- ✅ Error codes (433, 461, 464) correct
- ✅ No memory leaks (valgrind clean)
- ✅ No segmentation faults
- ✅ All test scripts pass
- ✅ RFC 1459 compliant

**SPRINT S2: COMPLETE** ✅
