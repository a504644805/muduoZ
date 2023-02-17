CXXFLAGS = -c -g -O0 -Wall -Wextra -Werror \
	   -Wconversion -Wno-unused-parameter \
	   -Wold-style-cast -Woverloaded-virtual \
	   -Wpointer-arith -Wshadow -Wwrite-strings \
	   -march=native -rdynamic

all: 
	g++ $(CXXFLAGS) CurrentThread.cc
	
	ar -crv libbase.a CurrentThread.o
	rm CurrentThread.o
	mv libbase.a ../../lib
.PHONY: all clean
