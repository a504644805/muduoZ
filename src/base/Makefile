CXXFLAGS = -c -g -O0 -Wall -Wextra -Werror \
	   -Wconversion -Wno-unused-parameter \
	   -Wold-style-cast -Woverloaded-virtual \
	   -Wpointer-arith -Wshadow -Wwrite-strings \
	   -march=native -rdynamic -lpthread

all: 
	g++ $(CXXFLAGS) CurrentThread.cc
	g++ $(CXXFLAGS) AsyncLogging.cc	
	g++ $(CXXFLAGS) Timestamp.cc
	g++ $(CXXFLAGS) Thread.cc
	#...
	ar -crv libbase.a CurrentThread.o AsyncLogging.o Timestamp.o Thread.o #...
	rm CurrentThread.o AsyncLogging.o Timestamp.o Thread.o #...
	mv libbase.a ../../lib
.PHONY: all clean
