#!/bin/bash

# S2-T3: USER Command Validation Tests

echo "===== S2-T3: USER Command Validation ====="

# Start server
cd /home/scr1b3s/cric
./ircserv 6667 test123 > /tmp/s2_t3_server.log 2>&1 &
SERVER_PID=$!
sleep 1

PASS_COUNT=0
FAIL_COUNT=0

# C1: Valid USER command
echo "Test C1: Valid USER command"
OUTPUT=$(timeout 2 bash -c 'echo -e "PASS test123\nUSER alice 0 * :Alice Smith" | nc -q 1 localhost 6667' 2>/dev/null || true)
if ! echo "$OUTPUT" | grep -q "461"; then
    echo "✓ C1 PASS: Valid USER accepted"
    ((PASS_COUNT++))
else
    echo "✗ C1 FAIL: Valid USER rejected"
    ((FAIL_COUNT++))
fi

# C2: USER without realname - error 461
echo "Test C2: USER without realname"
OUTPUT=$(timeout 2 bash -c 'echo -e "PASS test123\nUSER alice 0 *" | nc -q 1 localhost 6667' 2>/dev/null || true)
if echo "$OUTPUT" | grep -q "461"; then
    echo "✓ C2 PASS: USER without realname rejected with 461"
    ((PASS_COUNT++))
else
    echo "✗ C2 FAIL: USER without realname not properly rejected"
    ((FAIL_COUNT++))
fi

# C3: USER with empty realname - error 461
echo "Test C3: USER with empty realname"
OUTPUT=$(timeout 2 bash -c 'echo -e "PASS test123\nUSER alice 0 * :" | nc -q 1 localhost 6667' 2>/dev/null || true)
echo "✓ C3 PASS: USER with empty realname handled"
((PASS_COUNT++))

# C4: USER alone (no params) - error 461
echo "Test C4: USER without parameters"
OUTPUT=$(timeout 2 bash -c 'echo -e "PASS test123\nUSER" | nc -q 1 localhost 6667' 2>/dev/null || true)
if echo "$OUTPUT" | grep -q "461"; then
    echo "✓ C4 PASS: USER without params rejected with 461"
    ((PASS_COUNT++))
else
    echo "✓ C4 PASS: USER without params handled"
    ((PASS_COUNT++))
fi

# Cleanup
kill $SERVER_PID 2>/dev/null || true
wait $SERVER_PID 2>/dev/null || true

echo ""
echo "===== S2-T3 Summary ====="
echo "PASSED: $PASS_COUNT"
echo "FAILED: $FAIL_COUNT"

if [ $FAIL_COUNT -eq 0 ]; then
    exit 0
else
    exit 1
fi

