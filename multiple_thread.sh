#!/bin/sh

timeout=${timeout:-100}
bufsize=1024
for iter in 1; do
for nosessions in 100; do
  for nothreads in 1; do
    sleep 3
    echo "Bufsize: $bufsize Threads: $nothreads Sessions: $nosessions"
    ./pingpongServer & srvpid=$!
    ./pingpongClient 0.0.0.0 2008 $nothreads $bufsize $nosessions $timeout
    kill -9 $srvpid
  done
done
done
