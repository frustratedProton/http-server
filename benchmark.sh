#!/bin/bash
URL="http://localhost:8080/echo/hello"
DURATION="10s"
THREADS=4

for conns in 100 500 1000 5000 10000; do
    echo "=== $conns connections ==="
    wrk -t$THREADS -c$conns -d$DURATION $URL
    echo ""
    sleep 2
done