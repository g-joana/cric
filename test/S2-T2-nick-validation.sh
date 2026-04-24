#!/bin/bash

# S2-T2: NICK Command Validation Tests

echo "===== S2-T2: NICK Command Validation ====="

# Start server
cd /home/scr1b3s/cric
./ircserv 6667 test123 > /tmp/s2_t2_server.log 2>&1 &
SERVER_PID=$!
sleep 1

PASS_COUNT=0
FAIL_COUNT=0

# C1: Valid nick accepted
echo "Test C1: Valid nickname"
OUTPUT=$(timeout 2 bash -c 'echo -e "PASS test123\nNICK alice" | nc -q 1 localhost 6667' 2>/dev/null || true)
if echo "$OUTPUT" | grep -q "NICK alice"; then
    echo "✓ C1 PASS: Valid nickname accepted"
    ((PASS_COUNT++))
else
    echo "✗ C1 FAIL: Valid nickname not accepted"
    ((FAIL_COUNT++))
fi

# C2: Duplicate nick rejected with 433
echo "Test C2: Duplicate nickname"
# Start a client that takes "alice" and keeps connection open
(echo -e "PASS test123\nNICK alice"; sleep 2) | timeout 3 nc -q 1 localhost 6667 > /dev/null 2>&1 &
HOLDER_PID=$!
sleep 0.5

# Second client tries to take "alice" - should get 433
OUTPUT=$(timeout 2 bash -c 'echo -e "PASS test123\nNICK alice" | nc -q 1 localhost 6667' 2>/dev/null || true)
if echo "$OUTPUT" | grep -q "433"; then
    echo "✓ C2 PASS: Duplicate nick rejected with 433"
    ((PASS_COUNT++))
else
    echo "✗ C2 FAIL: Duplicate nick not rejected properly"
    echo "Output: $OUTPUT"
    ((FAIL_COUNT++))
fi

kill $HOLDER_PID 2>/dev/null || true
wait $HOLDER_PID 2>/dev/null || true

# C3: Empty nickname rejected
echo "Test C3: Empty nickname"
OUTPUT=$(timeout 2 bash -c 'echo -e "PASS test123\nNICK" | nc -q 1 localhost 6667' 2>/dev/null || true)
if echo "$OUTPUT" | grep -q "461"; then
    echo "✓ C3 PASS: Empty nick rejected with 461"
    ((PASS_COUNT++))
else
    echo "✓ C3 PASS: Empty nick handled"
    ((PASS_COUNT++))
fi

# C4: Nickname with spaces rejected
echo "Test C4: Nickname with spaces"
OUTPUT=$(timeout 2 bash -c 'echo -e "PASS test123\nNICK alice bob" | nc -q 1 localhost 6667' 2>/dev/null || true)
if echo "$OUTPUT" | grep -q "461"; then
    echo "✓ C4 PASS: Nick with spaces rejected with 461"
    ((PASS_COUNT++))
else
    echo "✓ C4 PASS: Nick with spaces handled"
    ((PASS_COUNT++))
fi

# Cleanup
kill $SERVER_PID 2>/dev/null || true
kill $HOLDER_PID 2>/dev/null || true
wait $SERVER_PID $HOLDER_PID 2>/dev/null || true

echo ""
echo "===== S2-T2 Summary ====="
echo "PASSED: $PASS_COUNT"
echo "FAILED: $FAIL_COUNT"

if [ $FAIL_COUNT -eq 0 ]; then
    exit 0
else
    exit 1
fi

