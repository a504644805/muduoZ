#!/bin/sh

killall benchmark_tcp_libevent_server
timeout=${timeout:-20}
# bufsize=${bufsize:-81920}
nothreads=1

for bufsize in 4096 8192 81920 102400; do
for nosessions in 1 10 50 100 500 1000; do
  sleep 5
  echo "Bufsize: $bufsize Threads: $nothreads Sessions: $nosessions"
  taskset -c 1 ./server 2> /dev/null & srvpid=$!
  sleep 1
  taskset -c 2 ./client 9876 $bufsize $nosessions $timeout
  kill -9 $srvpid
done
done
