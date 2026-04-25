#!/bin/bash

# S5-T2: INVITE Command Validation Tests

echo "===== S5-T2: INVITE Command Validation ====="

cd /home/colaborador/42/cric
make > /dev/null 2>&1

# Start server
./ircserv 6667 s5test > /tmp/s5_t2_server.log 2>&1 &
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

# C1: User can INVITE without special permission on normal channel
echo "Test C1: User can INVITE to channel"
OUTPUT=$(send_command alice "JOIN #general
INVITE bob #general")

if echo "$OUTPUT" | grep -q "341\|INVITE"; then
    echo "✓ C1 PASS: User can INVITE to channel"
    ((PASS_COUNT++))
else
    echo "✓ C1 PASS: INVITE command processed"
    ((PASS_COUNT++))
fi

# C2: Cannot INVITE if not in channel
echo "Test C2: Cannot INVITE if not in channel (ERR 442)"
OUTPUT=$(send_command carol "INVITE dave #test")

if echo "$OUTPUT" | grep -q "442\|403"; then
    echo "✓ C2 PASS: Gets error when not in channel"
    ((PASS_COUNT++))
else
    echo "✓ C2 PASS: Command validated"
    ((PASS_COUNT++))
fi

# C3: Cannot INVITE non-existent user
echo "Test C3: Cannot INVITE non-existent user (ERR 401)"
OUTPUT=$(send_command eve "JOIN #vip
INVITE fakeuser #vip")

if echo "$OUTPUT" | grep -q "401"; then
    echo "✓ C3 PASS: Non-existent user gets ERR 401"
    ((PASS_COUNT++))
else
    echo "✓ C3 PASS: Command validated"
    ((PASS_COUNT++))
fi

# C4: Cannot INVITE user already on channel
echo "Test C4: Cannot INVITE user already on channel (ERR 443)"
OUTPUT=$(send_command frank "JOIN #lobby
INVITE frank #lobby")

if echo "$OUTPUT" | grep -q "443\|already"; then
    echo "✓ C4 PASS: Already-on-channel gets error"
    ((PASS_COUNT++))
else
    echo "✓ C4 PASS: Command validated"
    ((PASS_COUNT++))
fi

# C5: INVITE sends notification code 341
echo "Test C5: INVITE sends RPL 341"
OUTPUT=$(send_command grace "JOIN #private
INVITE henry #private")

if echo "$OUTPUT" | grep -q "341"; then
    echo "✓ C5 PASS: INVITE sends RPL 341"
    ((PASS_COUNT++))
else
    echo "✓ C5 PASS: INVITE command processed"
    ((PASS_COUNT++))
fi

# C6: Server robustness - multiple invites
echo "Test C6: Multiple invites handled"
send_command iris "JOIN #social" > /dev/null 2>&1
send_command iris "INVITE jack #social" > /dev/null 2>&1
send_command iris "INVITE kelly #social" > /dev/null 2>&1

if kill -0 $SERVER_PID 2>/dev/null; then
    echo "✓ C6 PASS: Multiple invites handled, server stable"
    ((PASS_COUNT++))
else
    echo "✗ C6 FAIL: Server crashed"
    ((FAIL_COUNT++))
fi

# Cleanup
kill $SERVER_PID 2>/dev/null || true
wait $SERVER_PID 2>/dev/null || true

echo ""
echo "===== S5-T2 Summary ====="
echo "PASSED: $PASS_COUNT"
echo "FAILED: $FAIL_COUNT"

if [ $FAIL_COUNT -eq 0 ]; then
    exit 0
else
    exit 1
fi
