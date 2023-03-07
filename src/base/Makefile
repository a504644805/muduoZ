CXXFLAGS = -c -g -Wall -O0 -Wextra -Werror \
	   -Wconversion -Wno-unused-parameter \
	   -Wold-style-cast -Woverloaded-virtual \
	   -Wpointer-arith -Wshadow -Wwrite-strings \
	   -march=native -rdynamic -lpthread

all: 
	g++ $(CXXFLAGS) CurrentThread.cc
	g++ $(CXXFLAGS) AsyncLogging.cc	
	g++ $(CXXFLAGS) Timestamp.cc
	g++ $(CXXFLAGS) Thread.cc
	g++ $(CXXFLAGS) Logger.cc
	g++ $(CXXFLAGS) LogStream.cc
	#...
	ar -crv libbase.a CurrentThread.o AsyncLogging.o Timestamp.o Thread.o Logger.o LogStream.o #...
	rm CurrentThread.o AsyncLogging.o Timestamp.o Thread.o Logger.o LogStream.o #...
	mv libbase.a ../../lib
.PHONY: all clean
