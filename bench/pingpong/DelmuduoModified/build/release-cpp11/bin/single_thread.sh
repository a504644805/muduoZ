#!/bin/sh

killall server
timeout=${timeout:-100}
nothreads=1
port=9876
#for bufsize in 1024 4096 8192 16384; do
#for nosessions in 1 10 100 1000 10000; do
for bufsize in 81920; do
for nosessions in 1000; do
  echo "Bufsize: $bufsize Threads: $nothreads Sessions: $nosessions"
  taskset -c 1 ./pingpong_server 0.0.0.0 $port $nothreads 2> /dev/null & srvpid=$!
  sleep 1
  taskset -c 2 ./pingpong_client 0.0.0.0 $port $nothreads $bufsize $nosessions $timeout
  kill -9 $srvpid
done
done
