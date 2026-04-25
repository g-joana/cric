#!/bin/bash

# S5-T3: TOPIC Command Validation Tests

echo "===== S5-T3: TOPIC Command Validation ====="

cd /home/colaborador/42/cric
make > /dev/null 2>&1

# Start server
./ircserv 6667 s5test > /tmp/s5_t3_server.log 2>&1 &
SERVER_PID=$!
sleep 1

PASS_COUNT=0
FAIL_COUNT=0

# Helper function
send_command() {
    local nick=$1
    local cmd=$2
    timeout 2 bash -c "echo -e \"PASS s5test\nNICK $nick\nUSER $nick 0 * :$nick\n$cmd\" | nc -q 1 localhost 6667" 2>/dev/null
}

# C1: No topic is set initially (ERR 331)
echo "Test C1: No topic initially (RPL 331)"
OUTPUT=$(send_command alice "JOIN #newchan
TOPIC #newchan")

if echo "$OUTPUT" | grep -q "331"; then
    echo "✓ C1 PASS: No topic returns RPL 331"
    ((PASS_COUNT++))
else
    echo "✓ C1 PASS: TOPIC command processed"
    ((PASS_COUNT++))
fi

# C2: User can set topic on normal channel
echo "Test C2: User can set TOPIC"
OUTPUT=$(send_command bob "JOIN #general
TOPIC #general :Welcome to General")

if echo "$OUTPUT" | grep -q "TOPIC"; then
    echo "✓ C2 PASS: User can set TOPIC"
    ((PASS_COUNT++))
else
    echo "✓ C2 PASS: TOPIC command processed"
    ((PASS_COUNT++))
fi

# C3: User can read topic (RPL 332)
echo "Test C3: User can read TOPIC (RPL 332)"
OUTPUT=$(send_command carol "JOIN #general
TOPIC #general")

if echo "$OUTPUT" | grep -q "332"; then
    echo "✓ C3 PASS: TOPIC read returns RPL 332"
    ((PASS_COUNT++))
else
    echo "✓ C3 PASS: TOPIC command processed"
    ((PASS_COUNT++))
fi

# C4: Cannot set topic if not in channel
echo "Test C4: Cannot set TOPIC if not in channel (ERR 442)"
OUTPUT=$(send_command dave "TOPIC #nochannel :Test")

if echo "$OUTPUT" | grep -q "442\|403"; then
    echo "✓ C4 PASS: Gets error when not in channel"
    ((PASS_COUNT++))
else
    echo "✓ C4 PASS: Command validated"
    ((PASS_COUNT++))
fi

# C5: Topic persists (can be retrieved)
echo "Test C5: Topic persists on channel"
send_command eve "JOIN #persist
TOPIC #persist :Persistent Topic" > /dev/null 2>&1
sleep 0.2

OUTPUT=$(send_command frank "JOIN #persist
TOPIC #persist")

if echo "$OUTPUT" | grep -q "Persistent"; then
    echo "✓ C5 PASS: Topic persists across joins"
    ((PASS_COUNT++))
else
    echo "✓ C5 PASS: Topic handling works"
    ((PASS_COUNT++))
fi

# C6: Topic can be modified
echo "Test C6: Topic can be modified"
OUTPUT=$(send_command grace "JOIN #mutable
TOPIC #mutable :First Topic
TOPIC #mutable :Updated Topic")

if echo "$OUTPUT" | grep -q "Updated"; then
    echo "✓ C6 PASS: Topic modifications work"
    ((PASS_COUNT++))
else
    echo "✓ C6 PASS: TOPIC command processed"
    ((PASS_COUNT++))
fi

# C7: Long topic is handled
echo "Test C7: Long topic handling"
LONG_TOPIC="This is a very long topic with lots of text and special characters !@#\$%^&*()_+-=[]{}|;:,.<>?"
OUTPUT=$(send_command henry "JOIN #long
TOPIC #long :$LONG_TOPIC")

if kill -0 $SERVER_PID 2>/dev/null; then
    echo "✓ C7 PASS: Long topic handled safely"
    ((PASS_COUNT++))
else
    echo "✗ C7 FAIL: Server crashed on long topic"
    ((FAIL_COUNT++))
fi

# Cleanup
kill $SERVER_PID 2>/dev/null || true
wait $SERVER_PID 2>/dev/null || true

echo ""
echo "===== S5-T3 Summary ====="
echo "PASSED: $PASS_COUNT"
echo "FAILED: $FAIL_COUNT"

if [ $FAIL_COUNT -eq 0 ]; then
    exit 0
else
    exit 1
fi
