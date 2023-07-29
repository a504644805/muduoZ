#!/bin/sh

killall server
timeout=${timeout:-100}

for bufsize in 1024 4096 8192 16384; do
for nosessions in 1 10 100 1000 10000; do
  for nothreads in 1; do
	  sleep 1
	  echo "Bufsize: $bufsize Threads: $nothreads Sessions: $nosessions"
	  ./server 0.0.0.0 55555 $nothreads $bufsize & srvpid=$!
	  ./client 127.0.0.1 55555 $nothreads $bufsize $nosessions $timeout
	  kill -9 $srvpid
  done
done
done
