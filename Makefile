MUDUO_INCLUDE_1 = ./src/net
MUDUO_INCLUDE_2 = ./src/base
MUDUO_LIBRARY = ./lib
SRC = .

CXXFLAGS = -g -O0 -Wall -Wextra -Werror \
	   -Wconversion -Wno-unused-parameter \
	   -Wold-style-cast -Woverloaded-virtual \
	   -Wpointer-arith -Wshadow -Wwrite-strings \
	   -march=native -rdynamic \
	   -I$(MUDUO_INCLUDE_1) \
	   -I$(MUDUO_INCLUDE_2)

LDFLAGS = -L$(MUDUO_LIBRARY) -lnet -lbase -lpthread

all:
	make -C ./src/base
	make -C ./src/net
	g++ $(CXXFLAGS) -o pingpongServer pingpongServer.cc $(LDFLAGS)
	g++ $(CXXFLAGS) -o pingpongClient pingpongClient.cc $(LDFLAGS)

src: 	
	make -C ./src/base
	make -C ./src/net
this:
	g++ $(CXXFLAGS) -o pingpongServer pingpongServer.cc $(LDFLAGS)
	g++ $(CXXFLAGS) -o pingpongClient pingpongClient.cc $(LDFLAGS)

echo: 
	g++ $(CXXFLAGS) -o echo echoServer.cc $(LDFLAGS)

.PHONY: all clean
