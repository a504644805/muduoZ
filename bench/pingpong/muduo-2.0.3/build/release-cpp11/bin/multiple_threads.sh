#!/bin/sh

killall server
timeout=${timeout:-20}
#nothreads=4
port=9876
#for bufsize in 1024 4096 8192 16384; do
#for nosessions in 1 10 100 1000 10000; do
for bufsize in 4096; do
for nosessions in 100 ; do
for nothreads in 1 2 3 4 5 6 7 8; do
  echo "Bufsize: $bufsize Threads: $nothreads Sessions: $nosessions"
  #taskset -c 0,1,2,3 ./pingpong_server 0.0.0.0 $port $nothreads 2> /dev/null & srvpid=$!
  ./pingpong_server 0.0.0.0 $port $nothreads 2> /dev/null & srvpid=$!
  sleep 1
  ./pingpong_client 0.0.0.0 $port $nothreads $bufsize $nosessions $timeout
  #taskset -c 4,5,6,7 ./pingpong_client 0.0.0.0 $port $nothreads $bufsize $nosessions $timeout
  kill -9 $srvpid
done
done
done

