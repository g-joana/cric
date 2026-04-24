#!/bin/bash

# S2-T4: State Machine Validation Tests

echo "===== S2-T4: State Machine Validation ====="

# Start server
cd /home/scr1b3s/cric
./ircserv 6667 test123 > /tmp/s2_t4_server.log 2>&1 &
SERVER_PID=$!
sleep 1

PASS_COUNT=0
FAIL_COUNT=0

# C1: PASS first (INIT -> AUTH)
echo "Test C1: PASS transitions to AUTH"
OUTPUT=$(timeout 2 bash -c 'echo "PASS test123" | nc -q 1 localhost 6667' 2>/dev/null || true)
if ! echo "$OUTPUT" | grep -q "464"; then
    echo "✓ C1 PASS: PASS accepted (INIT -> AUTH)"
    ((PASS_COUNT++))
else
    echo "✗ C1 FAIL: PASS not accepted"
    ((FAIL_COUNT++))
fi

# C2: NICK after PASS (AUTH -> ID)
echo "Test C2: NICK after PASS (AUTH -> ID)"
OUTPUT=$(timeout 2 bash -c 'echo -e "PASS test123\nNICK alice" | nc -q 1 localhost 6667' 2>/dev/null || true)
if echo "$OUTPUT" | grep -q "NICK alice"; then
    echo "✓ C2 PASS: NICK accepted after PASS (AUTH -> ID)"
    ((PASS_COUNT++))
else
    echo "✗ C2 FAIL: NICK not accepted"
    ((FAIL_COUNT++))
fi

# C3: USER after NICK (ID -> REGISTERED)
echo "Test C3: USER after NICK and PASS (ID -> REGISTERED)"
OUTPUT=$(timeout 2 bash -c 'echo -e "PASS test123\nNICK alice\nUSER alice 0 * :Alice" | nc -q 1 localhost 6667' 2>/dev/null || true)
if echo "$OUTPUT" | grep -q "001 alice"; then
    echo "✓ C3 PASS: USER accepted and RPL_WELCOME sent (ID -> REGISTERED)"
    ((PASS_COUNT++))
else
    echo "✗ C3 FAIL: USER/RPL_WELCOME not processed correctly"
    echo "Output: $OUTPUT"
    ((FAIL_COUNT++))
fi

# C4: Full sequence PASS -> NICK -> USER
echo "Test C4: Full authentication sequence"
OUTPUT=$(timeout 2 bash -c 'echo -e "PASS test123\nNICK bob\nUSER bob 0 * :Bob Smith" | nc -q 1 localhost 6667' 2>/dev/null || true)
if echo "$OUTPUT" | grep -q "001 bob"; then
    echo "✓ C4 PASS: Full sequence works (INIT -> AUTH -> ID -> REGISTERED)"
    ((PASS_COUNT++))
else
    echo "✗ C4 FAIL: Full sequence failed"
    ((FAIL_COUNT++))
fi

# Cleanup
kill $SERVER_PID 2>/dev/null || true
wait $SERVER_PID 2>/dev/null || true

echo ""
echo "===== S2-T4 Summary ====="
echo "PASSED: $PASS_COUNT"
echo "FAILED: $FAIL_COUNT"

if [ $FAIL_COUNT -eq 0 ]; then
    exit 0
else
    exit 1
fi

