MUDUO_INCLUDE = ../base
CXXFLAGS = -c -g -O0 -Wall -Wextra -Werror \
	   -Wconversion -Wno-unused-parameter \
	   -Wold-style-cast -Woverloaded-virtual \
	   -Wpointer-arith -Wshadow -Wwrite-strings \
	   -march=native -rdynamic \
	   -I$(MUDUO_INCLUDE)

all: 
	g++ $(CXXFLAGS) EventLoop.cc
	g++ $(CXXFLAGS) Channel.cc
	g++ $(CXXFLAGS) Poller.cc
	g++ $(CXXFLAGS) TcpServer.cc
	g++ $(CXXFLAGS) Acceptor.cc
	g++ $(CXXFLAGS) TcpConnection.cc
	g++ $(CXXFLAGS) Buffer.cc
	g++ $(CXXFLAGS) SockAddrIn.cc
	g++ $(CXXFLAGS) Socket.cc
	#...
	ar -crv libnet.a EventLoop.o Channel.o Poller.o TcpServer.o Acceptor.o TcpConnection.o Buffer.o SockAddrIn.o Socket.o #...
	rm EventLoop.o Channel.o Poller.o TcpServer.o Acceptor.o TcpConnection.o Buffer.o SockAddrIn.o Socket.o #...
	mv libnet.a ../../lib
.PHONY: all clean
