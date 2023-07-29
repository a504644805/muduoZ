#!/bin/sh

killall server
timeout=${timeout:-20}
nothreads=1

#for bufsize in 1024 4096 8192 16384; do
#for nosessions in 1 10 100 1000 10000; do
for bufsize in 81920; do
for nosessions in 1000; do
  echo "Bufsize: $bufsize Threads: $nothreads Sessions: $nosessions"
  taskset -c 1 ./server 2> /dev/null & srvpid=$!
  sleep 1
  taskset -c 2 ./client 9876 $bufsize $nosessions $timeout
  kill -9 $srvpid
done
done
