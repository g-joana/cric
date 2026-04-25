# S5 Architecture Diagram

## System Architecture Overview

```
┌─────────────────────────────────────────────────────────────────┐
│                        IRC Server (ircserv)                      │
│                                                                  │
│  ┌────────────────────────────────────────────────────────────┐ │
│  │                      Poll Loop (main)                      │ │
│  │  - Non-blocking I/O with poll(2)                          │ │
│  │  - Handles server socket + all client sockets             │ │
│  └────────────────────────────────────────────────────────────┘ │
│                              │                                   │
│           ┌────────────────┬─┴─┬────────────────┐              │
│           │                │   │                │              │
│      New Client        Read Data        Write Data             │
│           │                │   │                │              │
│           ▼                ▼   │                ▼              │
│  ┌─────────────────┐   ┌──────┴───────────────────┐            │
│  │ _acceptClient  │   │  _processCommand          │            │
│  │                │   │  ┌────────────────────┐   │            │
│  └─────┬───────────┘   │  │ Dispatcher:        │   │            │
│        │               │  │ • PASS             │   │            │
│        │               │  │ • NICK             │   │            │
│        │               │  │ • USER             │   │            │
│        │               │  │ • PING             │   │            │
│        │               │  │ • PRIVMSG          │   │            │
│        │               │  │ • JOIN             │   │            │
│        │               │  │ • PART             │   │            │
│        │               │  │ • QUIT             │   │            │
│        │               │  │ • KICK      ← NEW  │   │            │
│        │               │  │ • INVITE    ← NEW  │   │            │
│        │               │  │ • TOPIC     ← NEW  │   │            │
│        │               │  │ • MODE      ← NEW  │   │            │
│        │               │  └────────────────────┘   │            │
│        │               └──────────┬────────────────┘            │
│  ┌─────▼─────────────┐           │                             │
│  │  Client {fd}      │      ┌────▼─────────┐                  │
│  │  • nickname       │      │ Call Handler │                  │
│  │  • user           │      └────┬─────────┘                  │
│  │  • realname       │           │                             │
│  │  • state          │      ┌────▼──────────────────────┐     │
│  │  • channels []    │      │  Handler (KICK, INVITE    │     │
│  │  • parser         │      │   TOPIC, MODE)            │     │
│  │  • isOp? (via ch) │      │  1. Validate permission ✓ │     │
│  └───────────────────┘      │  2. Parse parameters       │     │
│                             │  3. Apply changes          │     │
│                             └────┬─────────────────────┘      │
│                                  │                              │
│                             ┌────▼──────────────┐              │
│                             │ Channel updates   │              │
│                             │ • modes (i,t,k,o,l)              │
│                             │ • topic           │              │
│                             │ • members         │              │
│                             │ • operators       │              │
│                             │ • invited         │              │
│                             └────┬──────────────┘              │
│                                  │                              │
│                             ┌────▼──────────────┐              │
│                             │ Broadcast message │              │
│                             │ to all members     │              │
│                             └──────────────────┘              │
│                                                                 │
│  ┌─────────────────────────────────────────────────────────┐  │
│  │ Data Structures                                         │  │
│  │ • std::map<int, Client*> _clients                       │  │
│  │ • std::map<std::string, Channel*> _channels            │  │
│  │                                                         │  │
│  │ Each Channel has:                                      │  │
│  │ • std::map<int, Client*> _members                      │  │
│  │ • std::set<int> _operators                             │  │
│  │ • std::set<int> _invited                               │  │
│  │ • bool _inviteOnly (+i)                                │  │
│  │ • bool _topicRestricted (+t)                           │  │
│  │ • std::string _key (+k)                                │  │
│  │ • int _userLimit (+l)                                  │  │
│  └─────────────────────────────────────────────────────────┘  │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

---

## Permission Check Flow

```
Client sends: /kick #channel target
    │
    ▼
1. Is client registered?
    ├─ NO  → ERR 451 (not registered)
    └─ YES ▼
    
2. Parse command (extract channel, target)
    │
    ▼
3. Does channel exist?
    ├─ NO  → ERR 403 (no such channel)
    └─ YES ▼
    
4. Is client a member of channel?
    ├─ NO  → ERR 442 (not on that channel)
    └─ YES ▼
    
5. IS CLIENT CHANNEL OPERATOR? ✅ CRITICAL CHECK
    ├─ NO  → ERR 482 (ERR_CHANOPRIVSNEEDED) ← S5 KEY
    └─ YES ▼
    
6. Does target user exist?
    ├─ NO  → ERR 401 (no such nick)
    └─ YES ▼
    
7. Is target member of channel?
    ├─ NO  → ERR 441 (not on that channel)
    └─ YES ▼
    
8. PERFORM ACTION
    ├─ Remove from channel
    ├─ Broadcast to all members
    ├─ Clean up channel if empty
    └─ ✅ SUCCESS
```

---

## Mode Management State Machine

```
                    CHANNEL
                       │
        ┌──────────────┼──────────────┐
        │              │              │
        ▼              ▼              ▼
    Invite-Only    Topic-Restricted  Password
      (-i flag)      (-t flag)       (-k flag)
        │              │              │
    default:       default:       default:
    FALSE          FALSE          ""(empty)
        │              │              │
        │ +i            │ +t           │ +k secretpass
        └─→ TRUE        └─→ TRUE       └─→ "secretpass"
        │ -i            │ -t           │ -k
        └─→ FALSE       └─→ FALSE      └─→ ""
        
        ┌──────────────┬──────────────┐
        │              │              │
        ▼              ▼              ▼
     Operator      User Limit    [Can combine]
      (-o flag)     (-l flag)
        │              │
    default:       default:
    empty set      0(unlimited)
        │              │
   +o alice         +l 50
   target is op  limit = 50
        │              │
   -o alice         -l
   target=regular  unlimited
```

---

## Command Handler Relationship

```
┌──────────────────────────────────┐
│     _processCommand()             │
│     (Dispatcher)                  │
└──────────────────────────────────┘
            │
    ┌───────┼───────────────┬───────────────┐
    │       │               │               │
    ▼       ▼               ▼               ▼
 KICK    INVITE          TOPIC           MODE
  │        │               │               │
  │        │               │      ┌────────┴────────┐
  │        │               │      │                 │
  │        │               │   ┌──┴──┐           ┌──┴──┐
  │        │               │   │ +i  │...        │ +l  │
  │        │               │   │ -t  │           │ -k  │
  │        │               │   │ +o  │           │ +l  │
  │        │               │   │ +k  │           │ -o  │
  │        │               │   └─────┘           └─────┘
  │        │               │
  ▼        ▼               ▼
KICK    INVITE         TOPIC
Query  Query          Query
Channel Members      Channel
  │        │          Members
  │        │             │
  │        ▼              ▼
  │   Check +i?      Check +t?
  │   Grant op?      Grant op?
  │        
  ▼
Remove
Member

   │        │             │
   └────────┴─────────────┘
            │
            ▼
    broadcast()
       to
    all members
```

---

## Data Flow Example: MODE +i

```
Client (alice, operator):
    /mode #vip +i
        │
        ▼
CommandParser extracts: "MODE #vip +i"
        │
        ▼
Server::_processCommand() → dispatcher
        │
        ▼
Server::_handleMODE(alice, "#vip +i")
        │
    ┌───┴─────────────────────────┬────────────────┐
    │                             │                │
    ▼                             ▼                ▼
Validate:                    Parse modes:     Apply changes:
• alice registered ✓         • find '#'       • channel->
• alice on #vip ✓            • extract       setInviteOnly(true)
• alice is op ✓                  "+i"
• #vip exists ✓              • NO params
                             needed
    │                             │                │
    └─────────────────────────────┴────────────────┘
                    │
                    ▼
        Broadcast to all members:
        ":alice!alice@server MODE #vip +i"
                    │
        ┌───────────┼───────────┬─────────┐
        │           │           │         │
        ▼           ▼           ▼         ▼
      alice      bob(op)    charlie   diana(op)
      
        All receive: ":alice!alice@server MODE #vip +i"
        
        From now on:
        • New JOIN #vip requests need INVITE
        • Existing members unaffected
        • Mode can be toggled: /mode #vip -i
```

---

## Integration Points with Previous Sprints

```
Authentication (S2)
├─ PASS validation ✓
├─ NICK uniqueness ✓
├─ USER registration ✓
└─ State machine (REGISTERED) ✓
        │
        └─→ Required for ALL S5 commands
        
JOIN/PART (S3)
├─ Channel creation ✓
├─ Member tracking ✓
├─ First user = operator ✓
└─ Broadcast to members ✓
        │
        └─→ Foundation for S5 permission checks
        
PRIVMSG (S3)
├─ Broadcast system ✓
├─ Per-member messaging ✓
└─ Error handling ✓
        │
        └─→ Reused for S5 broadcasts
        
S5 - New Operators Layer
├─ Permission system (above JOIN/PART)
├─ Mode management (above Channel data)
├─ Permission-gated broadcasts
└─ Enhanced error codes

No breaking changes to existing code ✓
```

---

## Error Code Reference (S5 specific)

```
┌──────┬──────────────────────────────┬────────────────────┐
│ Code │ Constant (RFC)               │ When Used          │
├──────┼──────────────────────────────┼────────────────────┤
│ 401  │ ERR_NOSUCHNICK               │ User not found     │
│ 403  │ ERR_NOSUCHCHANNEL            │ Channel not found  │
│ 441  │ ERR_USERNOTINCHANNEL         │ Target not in chan │
│ 442  │ ERR_NOTONCHANNEL             │ Sender not in chan │
│ 443  │ ERR_USERONCHANNEL            │ Already on channel │
│ 451  │ ERR_NOTREGISTERED            │ Not authenticated  │
│ 461  │ ERR_NEEDMOREPARAMS           │ Missing parameters │
│ 482  │ ERR_CHANOPRIVSNEEDED ⭐     │ Need OPERATOR      │
│ 501  │ ERR_UMODEUNKNOWNFLAG         │ Invalid parameter  │
└──────┴──────────────────────────────┴────────────────────┘

⭐ = Critical for S5 permission model
```

---

## Performance Considerations

```
Each command O(complexity):

KICK
├─ Find channel: O(log n) hash
├─ Find target: O(n) linear scan
├─ Broadcast: O(m) = members count
└─ Total: O(n + m)

INVITE
├─ Find channel: O(log n) hash
├─ Find target: O(n) linear scan
├─ Add to set: O(log k) = O(k is invites)
└─ Total: O(n + log k)

TOPIC
├─ Find channel: O(log n) hash
├─ String assignment: O(p) = topic length
└─ Broadcast: O(m)
└─ Total: O(m + p)

MODE (worst case - parse all 5 modes)
├─ Find channel: O(log n)
├─ Parse 5 modes: O(5) = constant
├─ Apply each: O(log k) to O(n) depending
└─ Broadcast: O(m)
└─ Total: O(n + m)

With typical:
- n = 100 clients
- m = 10 members per channel
- All operations < 1ms
```
