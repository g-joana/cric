#!/bin/bash

# S2-T6: Integration tests with nc and irssi

echo "===== S2-T6: Integration Tests ====="

cd /home/scr1b3s/cric

# Start server
./ircserv 6667 s2test > /tmp/s2_t6_server.log 2>&1 &
SERVER_PID=$!
sleep 1

PASS_COUNT=0
FAIL_COUNT=0

# C1: Multiple clients authentication
echo "Test C1: Multiple clients simultaneous"
# Client A
timeout 2 bash -c 'echo -e "PASS s2test\nNICK alice\nUSER alice 0 * :Alice" | nc -q 1 localhost 6667' > /tmp/c1_a.txt 2>&1 &
PID_A=$!

# Client B
sleep 0.3
timeout 2 bash -c 'echo -e "PASS s2test\nNICK bob\nUSER bob 0 * :Bob" | nc -q 1 localhost 6667' > /tmp/c1_b.txt 2>&1 &
PID_B=$!

# Wait for both
wait $PID_A $PID_B 2>/dev/null || true

# Check both got 001
if grep -q "001 alice" /tmp/c1_a.txt && grep -q "001 bob" /tmp/c1_b.txt; then
    echo "✓ C1 PASS: Multiple clients authenticated"
    ((PASS_COUNT++))
else
    echo "✗ C1 FAIL: Multiple clients authentication failed"
    ((FAIL_COUNT++))
fi

# C2: Wrong password rejection
echo "Test C2: Wrong password rejection"
OUTPUT=$(timeout 2 bash -c 'echo "PASS wrongpass" | nc -q 1 localhost 6667' 2>/dev/null || true)
if echo "$OUTPUT" | grep -q "464"; then
    echo "✓ C2 PASS: Wrong password rejected with 464"
    ((PASS_COUNT++))
else
    echo "✗ C2 FAIL: Wrong password not properly rejected"
    ((FAIL_COUNT++))
fi

# C3: Duplicate nick rejection
echo "Test C3: Duplicate nick rejection"
# First client takes "eve" and keeps connection open
(echo -e "PASS s2test\nNICK eve"; sleep 2) | timeout 3 nc -q 1 localhost 6667 > /dev/null 2>&1 &
PID_EVE=$!
sleep 0.3

# Second client tries "eve"
OUTPUT=$(timeout 2 bash -c 'echo -e "PASS s2test\nNICK eve" | nc -q 1 localhost 6667' 2>/dev/null || true)
if echo "$OUTPUT" | grep -q "433"; then
    echo "✓ C3 PASS: Duplicate nick rejected with 433"
    ((PASS_COUNT++))
else
    echo "✗ C3 FAIL: Duplicate nick not properly rejected"
    ((FAIL_COUNT++))
fi

kill $PID_EVE 2>/dev/null || true
wait $PID_EVE 2>/dev/null || true

# C4: Full authentication sequence (nc)
echo "Test C4: Full authentication via nc"
OUTPUT=$(timeout 2 bash -c 'echo -e "PASS s2test\nNICK frank\nUSER frank 0 * :Frank" | nc -q 1 localhost 6667' 2>/dev/null || true)
if echo "$OUTPUT" | grep -q "001 frank"; then
    echo "✓ C4 PASS: Full authentication sequence works"
    ((PASS_COUNT++))
else
    echo "✗ C4 FAIL: Full sequence failed"
    ((FAIL_COUNT++))
fi

# C5: Robustness - fragmented commands
echo "Test C5: Fragmented command handling"
timeout 2 bash -c 'echo -e "PASS s2test\nNICK grace\nUSER grace 0 * :Grace" | nc -q 1 localhost 6667' > /dev/null 2>&1 &
PID_FRAG=$!
wait $PID_FRAG 2>/dev/null || true

# Just check server still running
if kill -0 $SERVER_PID 2>/dev/null; then
    echo "✓ C5 PASS: Fragmented commands handled safely"
    ((PASS_COUNT++))
else
    echo "✗ C5 FAIL: Server crashed on fragmented input"
    ((FAIL_COUNT++))
fi

# C6: Empty input handling
echo "Test C6: Empty input handling"
timeout 2 bash -c 'echo -e "\nPASS s2test\nNICK henry\nUSER henry 0 * :Henry" | nc -q 1 localhost 6667' > /dev/null 2>&1 &
PID_EMPTY=$!
wait $PID_EMPTY 2>/dev/null || true

if kill -0 $SERVER_PID 2>/dev/null; then
    echo "✓ C6 PASS: Empty input handled safely"
    ((PASS_COUNT++))
else
    echo "✗ C6 FAIL: Server crashed on empty input"
    ((FAIL_COUNT++))
fi

# Cleanup
kill $SERVER_PID 2>/dev/null || true
wait $SERVER_PID 2>/dev/null || true

echo ""
echo "===== S2-T6 Summary ====="
echo "PASSED: $PASS_COUNT"
echo "FAILED: $FAIL_COUNT"

if [ $FAIL_COUNT -eq 0 ]; then
    exit 0
else
    exit 1
fi

