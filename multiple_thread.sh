#!/bin/sh

timeout=${timeout:-300}
bufsize=1024

for nosessions in 1; do
  for nothreads in 1; do
    sleep 3
    echo "Bufsize: $bufsize Threads: $nothreads Sessions: $nosessions"
    ./pingpongServer & srvpid=$!
    ./pingpongClient 0.0.0.0 2008 $nothreads $bufsize $nosessions $timeout
    kill -9 $srvpid
  done
done
