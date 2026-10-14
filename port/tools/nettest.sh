#!/bin/sh
# Tests the native port's multiplayer without game data: several `gta2 --nettest` processes play
# against each other on this machine through the port's DirectPlay (port/platform/nettest.cpp).
#
#   port/tools/nettest.sh BUILD_DIR [OTHER_BUILD_DIR]
#
# BUILD_DIR holds gta2 and gta2-server (e.g. build_port/linux32). With a second build (e.g. the 64-bit
# one) the players are mixed across the two, and their --simcheck output must match: that is what lets
# different systems play together.
set -u
a="$1"
b="${2:-$1}"
work=$(mktemp -d)
trap 'kill $(jobs -p) 2>/dev/null; rm -rf "$work"' EXIT
failed=0
port=23100

next_port() {
    port=$((port + 7))
}

# run NAME BUILD ARGS...: starts one player in the background, output in $work/NAME.log
run() {
    name=$1
    build=$2
    shift 2
    "$build/gta2" --nettest "$@" --name "$name" >"$work/$name.log" 2>&1 &
    eval "pid_$name=$!"
}

# expect NAME: waits for that player and checks it said OK
expect() {
    eval "pid=\$pid_$1"
    if wait "$pid" && grep -q ": OK" "$work/$1.log"; then
        return 0
    fi
    echo "  $1 failed:" && sed 's/^/    /' "$work/$1.log"
    return 1
}

scenario() {
    printf '%s ... ' "$1"
}

result() {
    if [ "$1" = 0 ]; then
        echo "ok"
    else
        echo "FAILED"
        failed=1
    fi
}

wait_for() { # wait_for FILE TEXT: until a log line appears (10 s at most)
    i=0
    while ! grep -q "$2" "$1" 2>/dev/null; do
        i=$((i + 1))
        [ $i -gt 200 ] && return 1
        sleep 0.05
    done
}

scenario "two players, found on the LAN"
next_port
run host "$a" host --players 2 --frames 300 --port $port
run p1 "$b" join --frames 300 --port $port
r=0; expect host || r=1; expect p1 || r=1; result $r

scenario "four players, 25% of datagrams lost"
next_port
export GTA2_NET_DROP=25
run host "$a" host --players 4 --frames 200 --port $port
run p1 "$b" join --frames 200 --port $port --address 127.0.0.1:$port
run p2 "$a" join --frames 200 --port $port --address 127.0.0.1:$port
run p3 "$b" join --frames 200 --port $port --address 127.0.0.1:$port
r=0; for p in host p1 p2 p3; do expect $p || r=1; done; result $r
unset GTA2_NET_DROP

scenario "a player leaves during the game"
next_port
run host "$a" host --players 3 --frames 300 --port $port
run p1 "$b" join --frames 300 --port $port --address 127.0.0.1:$port
run p2 "$a" join --frames 300 --port $port --address 127.0.0.1:$port --leave-after 100
r=0; for p in host p1 p2; do expect $p || r=1; done
grep -q "(1 left during the game)" "$work/host.log" || { echo "  the host didn't see p2 leave"; r=1; }
result $r

scenario "the host quits: the others' session ends at once"
next_port
run host "$a" host --players 2 --frames 100 --port $port
run p1 "$b" join --frames 100000 --port $port --expect-lost
r=0; expect host || r=1; expect p1 || r=1; result $r

scenario "the host's machine dies: the session ends after the timeout"
next_port
export GTA2_NET_TIMEOUT_MS=3000
run host "$a" host --players 2 --frames 100000 --port $port
run p1 "$b" join --frames 100000 --port $port --expect-lost
if wait_for "$work/p1.log" "playing"; then
    kill -9 "$pid_host" 2>/dev/null
    wait "$pid_host" 2>/dev/null
    r=0; expect p1 || r=1
else
    r=1; cat "$work/p1.log"
fi
result $r
unset GTA2_NET_TIMEOUT_MS

scenario "a different build is turned away, saying why"
next_port
run host "$a" host --players 2 --frames 100000 --port $port
GTA2_NET_TEST_COMPAT=another-build run p1 "$b" join --port $port --expect-refused
r=0; expect p1 || r=1; kill "$pid_host" 2>/dev/null; wait "$pid_host" 2>/dev/null; result $r

scenario "three players through a separate gta2-server"
next_port
"$a/gta2-server" --port $port --quiet >"$work/server.log" 2>&1 &
server=$!
sleep 0.2
run host "$b" host --players 3 --frames 200 --server 127.0.0.1:$port --port $((port + 1))
run p1 "$a" join --frames 200 --address 127.0.0.1:$port
run p2 "$b" join --frames 200 --address 127.0.0.1:$port
r=0; for p in host p1 p2; do expect $p || r=1; done
kill "$server"; wait "$server" 2>/dev/null
result $r

if [ "$a" != "$b" ]; then
    scenario "the two builds compute the same simulation"
    "$a/gta2" --simcheck >"$work/sim_a.log" 2>&1
    "$b/gta2" --simcheck >"$work/sim_b.log" 2>&1
    if cmp -s "$work/sim_a.log" "$work/sim_b.log"; then
        result 0
    else
        diff "$work/sim_a.log" "$work/sim_b.log" | sed 's/^/    /'
        result 1
    fi
fi

if [ $failed = 0 ]; then
    echo "All network tests passed"
else
    echo "Network tests FAILED"
fi
exit $failed
