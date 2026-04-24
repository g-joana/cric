#!/bin/bash

# S2-T5: RPL_WELCOME (001) Tests

echo "===== S2-T5: RPL_WELCOME (001) ====="

# Start server
cd /home/scr1b3s/cric
./ircserv 6667 test123 > /tmp/s2_t5_server.log 2>&1 &
SERVER_PID=$!
sleep 1

PASS_COUNT=0
FAIL_COUNT=0

# C1: RPL_WELCOME (001) received
echo "Test C1: RPL_WELCOME code 001"
OUTPUT=$(timeout 2 bash -c 'echo -e "PASS test123\nNICK alice\nUSER alice 0 * :Alice" | nc -q 1 localhost 6667' 2>/dev/null || true)
if echo "$OUTPUT" | grep -q "001 alice"; then
    echo "✓ C1 PASS: RPL_WELCOME (001) code present"
    ((PASS_COUNT++))
else
    echo "✗ C1 FAIL: RPL_WELCOME (001) not found"
    echo "Output: $OUTPUT"
    ((FAIL_COUNT++))
fi

# C2: Welcome message contains "Welcome"
echo "Test C2: Welcome message text"
OUTPUT=$(timeout 2 bash -c 'echo -e "PASS test123\nNICK bob\nUSER bob 0 * :Bob" | nc -q 1 localhost 6667' 2>/dev/null || true)
if echo "$OUTPUT" | grep -qi "welcome"; then
    echo "✓ C2 PASS: Welcome message text present"
    ((PASS_COUNT++))
else
    echo "✗ C2 FAIL: Welcome message not found"
    echo "Output: $OUTPUT"
    ((FAIL_COUNT++))
fi

# C3: RPL_WELCOME format includes nickname
echo "Test C3: RPL_WELCOME includes nickname"
OUTPUT=$(timeout 2 bash -c 'echo -e "PASS test123\nNICK charlie\nUSER charlie 0 * :Charlie" | nc -q 1 localhost 6667' 2>/dev/null || true)
if echo "$OUTPUT" | grep -q "001 charlie"; then
    echo "✓ C3 PASS: RPL_WELCOME includes correct nickname"
    ((PASS_COUNT++))
else
    echo "✗ C3 FAIL: RPL_WELCOME nickname mismatch"
    ((FAIL_COUNT++))
fi

# C4: RPL_WELCOME sent only when fully registered
echo "Test C4: RPL_WELCOME only when registered"
# After PASS and NICK (but no USER), should NOT have 001 yet
OUTPUT=$(timeout 2 bash -c 'echo -e "PASS test123\nNICK dave" | nc -q 1 localhost 6667' 2>/dev/null || true)
if ! echo "$OUTPUT" | grep -q "001 dave"; then
    echo "✓ C4 PASS: RPL_WELCOME not sent before USER"
    ((PASS_COUNT++))
else
    echo "✗ C4 FAIL: RPL_WELCOME sent too early"
    ((FAIL_COUNT++))
fi

# Cleanup
kill $SERVER_PID 2>/dev/null || true
wait $SERVER_PID 2>/dev/null || true

echo ""
echo "===== S2-T5 Summary ====="
echo "PASSED: $PASS_COUNT"
echo "FAILED: $FAIL_COUNT"

if [ $FAIL_COUNT -eq 0 ]; then
    exit 0
else
    exit 1
fi

