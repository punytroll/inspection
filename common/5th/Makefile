CXXFLAGS = -std=c++14 -Wall -Werror -g -O0
LDFLAGS = -g

default: all

clean:
	$(RM) test
	$(RM) test.o

all: test

test: test.o
	$(CXX) $(LDFLAGS) $^ -o $@

test.o: test.cpp buffer.h length.h result.h value.h
	$(CXX) $(CXXFLAGS) -c $< -o $@
