#!/bin/bash

PORT=6667
PASS=test123
FAILED=0

echo "=== S4 Acceptance Test ==="

pkill -9 ircserv 2>/dev/null
sleep 1

./ircserv $PORT $PASS > /dev/null 2>&1 &
SERVER_PID=$!
sleep 2

echo "=== Alice joins ==="
(
    echo "PASS $PASS"
    echo "NICK alice"
    echo "USER alice 0 * :Alice"
    sleep 1
    echo "JOIN #test"
    sleep 3
) | nc -q 2 localhost $PORT > /tmp/alice.log 2>&1 &

sleep 1

echo "=== Bob joins, sends msg, parts ==="
(
    echo "PASS $PASS"
    echo "NICK bob"
    echo "USER bob 0 * :Bob"
    sleep 1
    echo "JOIN #test"
    sleep 1
    echo "PRIVMSG #test :Hi"
    sleep 2
    echo "PART #test"
    sleep 2
) | nc -q 2 localhost $PORT > /tmp/bob.log 2>&1 &

sleep 4

kill $SERVER_PID 2>/dev/null
sleep 1

echo "=== Results ==="

grep -q "#test" /tmp/alice.log && echo "✓ T1: JOIN" || FAILED=1
grep -q "bob.*JOIN" /tmp/alice.log && echo "✓ T2: Broadcast JOIN" || FAILED=1
grep -q "PRIVMSG #test" /tmp/alice.log && echo "✓ T3: Channel PRIVMSG" || FAILED=1

rm -f /tmp/alice.log /tmp/bob.log

[ $FAILED -eq 0 ] && echo "=== All Passed ===" || echo "=== Failed ==="
exit $FAILED