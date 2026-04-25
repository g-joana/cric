#!/bin/bash
# S6 Acceptance Tests

PORT=6667
PASS="password"
TESTS_PASSED=0
TESTS_FAILED=0

cleanup() {
    if [ -n "$SERVER_PID" ]; then
        kill $SERVER_PID 2>/dev/null
        wait $SERVER_PID 2>/dev/null
    fi
}
trap cleanup EXIT

cd /home/scr1b3s/cric

echo "=== S6 Acceptance Tests ==="

# Test 1: Server starts
echo -n "Test 1: Server starts: "
./ircserv $PORT $PASS 2>/dev/null &
SERVER_PID=$!
sleep 1
if kill -0 $SERVER_PID 2>/dev/null; then
    echo "PASS"
    TESTS_PASSED=$((TESTS_PASSED+1))
else
    echo "FAIL"
    TESTS_FAILED=$((TESTS_FAILED+1))
fi
kill $SERVER_PID 2>/dev/null
wait $SERVER_PID 2>/dev/null
sleep 1

# Test 2: SIGINT cleanup
echo -n "Test 2: SIGINT cleanup: "
./ircserv $PORT $PASS 2>/dev/null &
SERVER_PID=$!
sleep 1
kill -INT $SERVER_PID
sleep 1
if ! kill -0 $SERVER_PID 2>/dev/null; then
    echo "PASS"
    TESTS_PASSED=$((TESTS_PASSED+1))
else
    echo "FAIL"
    TESTS_FAILED=$((TESTS_FAILED+1))
fi
wait $SERVER_PID 2>/dev/null
sleep 1

# Test 3: JOIN works
echo -n "Test 3: JOIN works: "
./ircserv $PORT $PASS &
SERVER_PID=$!
sleep 1
(echo "PASS $PASS"; echo "NICK user1"; echo "USER u 0 * :U"; sleep 1
 echo "JOIN #test"; echo "QUIT"
) | nc -q 2 localhost $PORT > /tmp/s6_join.log 2>&1 &
sleep 2
if grep -q "JOIN #test" /tmp/s6_join.log; then
    echo "PASS"
    TESTS_PASSED=$((TESTS_PASSED+1))
else
    echo "FAIL"
    TESTS_FAILED=$((TESTS_FAILED+1))
fi
kill $SERVER_PID 2>/dev/null
wait $SERVER_PID 2>/dev/null
sleep 1

# Test 4: PRIVMSG works
echo -n "Test 4: PRIVMSG works: "
./ircserv $PORT $PASS &
SERVER_PID=$!
sleep 1
(echo "PASS $PASS"; echo "NICK user1"; echo "USER u 0 * :U"; sleep 1
 echo "JOIN #test"; sleep 1
 echo "PRIVMSG #test :Hello"; sleep 1
 echo "QUIT"
) | nc -q 2 localhost $PORT > /tmp/s6_privmsg.log 2>&1 &
sleep 3
if grep -q "PRIVMSG" /tmp/s6_privmsg.log; then
    echo "PASS"
    TESTS_PASSED=$((TESTS_PASSED+1))
else
    echo "FAIL"
    TESTS_FAILED=$((TESTS_FAILED+1))
fi
kill $SERVER_PID 2>/dev/null
wait $SERVER_PID 2>/dev/null
sleep 1

# Test 5: MODE +i blocks join
echo -n "Test 5: MODE +i blocks: "
./ircserv $PORT $PASS &
SERVER_PID=$!
sleep 1
(echo "PASS $PASS"; echo "NICK alice"; echo "USER a 0 * :A"; sleep 1
 echo "JOIN #secret"; sleep 2
 echo "MODE #secret +i"; sleep 2
 echo "QUIT"
) | nc -q 2 localhost $PORT > /tmp/s6_mode_i.log 2>&1 &
ALICE_PID=$!
sleep 3
(echo "PASS $PASS"; echo "NICK bob"; echo "USER b 0 * :B"; sleep 1
 echo "JOIN #secret"; sleep 1
 echo "QUIT"
) | nc -q 2 localhost $PORT > /tmp/s6_mode_i2.log 2>&1 &
sleep 3
if grep -q "473" /tmp/s6_mode_i2.log; then
    echo "PASS"
    TESTS_PASSED=$((TESTS_PASSED+1))
else
    echo "FAIL"
    TESTS_FAILED=$((TESTS_FAILED+1))
fi
kill $SERVER_PID $ALICE_PID 2>/dev/null
wait $SERVER_PID $ALICE_PID 2>/dev/null
sleep 1

# Test 6: KICK works
echo -n "Test 6: KICK works: "
./ircserv $PORT $PASS &
SERVER_PID=$!
sleep 1
(echo "PASS $PASS"; echo "NICK alice"; echo "USER a 0 * :A"; sleep 1
 echo "JOIN #kick"; sleep 1
 echo "KICK #kick alice"; sleep 1
 echo "QUIT"
) | nc -q 2 localhost $PORT > /tmp/s6_kick.log 2>&1 &
sleep 2
if grep -q "KICK" /tmp/s6_kick.log; then
    echo "PASS"
    TESTS_PASSED=$((TESTS_PASSED+1))
else
    echo "FAIL"
    TESTS_FAILED=$((TESTS_FAILED+1))
fi
kill $SERVER_PID 2>/dev/null
wait $SERVER_PID 2>/dev/null
sleep 1

echo ""
echo "=== Results ==="
echo "Passed: $TESTS_PASSED"
echo "Failed: $TESTS_FAILED"

if [ $TESTS_FAILED -eq 0 ]; then
    exit 0
else
    exit 1
fi