#!/bin/sh

#set -x

killall asio_test
timeout=${timeout:-100}

for bufsize in 1024 4096 8192 16384; do
for nosessions in 1 10 100 1000 10000; do
for nothreads in 1; do
  echo "======================> (test1) Bufsize: $bufsize Threads: $nothreads Sessions: $nosessions"
  sleep 1
  ./asio_test server1 127.0.0.1 33333 $nothreads $bufsize & srvpid=$!
  sleep 1
  ./asio_test client1 127.0.0.1 33333 $nothreads $bufsize $nosessions $timeout
  sleep 1
  kill -9 $srvpid
  sleep 3
done
done
done

#for nosessions in 100 1000; do
#for nothreads in 2 3 4 6 8; do
#  echo "======================> (test2) Bufsize: $bufsize Threads: $nothreads Sessions: $nosessions"
#  sleep 1
#  ./asio_test.exe server2 127.0.0.1 33333 $nothreads $bufsize & srvpid=$!
#  sleep 1
#  ./asio_test.exe client2 127.0.0.1 33333 $nothreads $bufsize $nosessions $timeout
#  sleep 1
#  kill -9 $srvpid
#  sleep 5
#done
#done
