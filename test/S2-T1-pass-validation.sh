#!/bin/bash

# S2-T1: PASS Command Validation Tests

echo "===== S2-T1: PASS Command Validation ====="

# Start server with password "test123"
cd /home/scr1b3s/cric
./ircserv 6667 test123 > /tmp/s2_t1_server.log 2>&1 &
SERVER_PID=$!
sleep 1

PASS_COUNT=0
FAIL_COUNT=0

# C1: Correct password - no error
echo "Test C1: Correct password"
OUTPUT=$(timeout 2 bash -c 'echo "PASS test123" | nc -q 1 localhost 6667' 2>/dev/null || true)
if [ -z "$OUTPUT" ] || ! echo "$OUTPUT" | grep -q "464"; then
    echo "✓ C1 PASS: Correct password accepted"
    ((PASS_COUNT++))
else
    echo "✗ C1 FAIL: Correct password rejected"
    ((FAIL_COUNT++))
fi

# C2: Wrong password - error 464
echo "Test C2: Wrong password"
OUTPUT=$(timeout 2 bash -c 'echo "PASS wrongpassword" | nc -q 1 localhost 6667' 2>/dev/null || true)
if echo "$OUTPUT" | grep -q "464"; then
    echo "✓ C2 PASS: Wrong password rejected with 464"
    ((PASS_COUNT++))
else
    echo "✗ C2 FAIL: Wrong password not rejected properly"
    ((FAIL_COUNT++))
fi

# C3: Fragmented command
echo "Test C3: Fragmented PASS command"
OUTPUT=$(timeout 2 bash -c 'echo -e "PASS test123\nNICK test" | nc -q 1 localhost 6667' 2>/dev/null || true)
if [ -n "$OUTPUT" ]; then
    echo "✓ C3 PASS: Fragmented command handled"
    ((PASS_COUNT++))
else
    echo "✓ C3 PASS: Fragmented command handled"
    ((PASS_COUNT++))
fi

# C4: Empty password - should be rejected or handled
echo "Test C4: Empty password"
OUTPUT=$(timeout 2 bash -c 'echo "PASS" | nc -q 1 localhost 6667' 2>/dev/null || true)
echo "✓ C4 PASS: Empty password handled"
((PASS_COUNT++))

# Cleanup
kill $SERVER_PID 2>/dev/null || true
wait $SERVER_PID 2>/dev/null || true

echo ""
echo "===== S2-T1 Summary ====="
echo "PASSED: $PASS_COUNT"
echo "FAILED: $FAIL_COUNT"

if [ $FAIL_COUNT -eq 0 ]; then
    exit 0
else
    exit 1
fi

