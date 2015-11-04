CXXFLAGS = -std=c++14 -Wall -Werror -g -O0

default: all

clean:
	$(RM) ../id3inspector
	$(RM) id3inspector.o

all: ../id3inspector

../id3inspector: id3inspector.o
	$(CXX) $(LDFLAGS) $^ -o $@

id3inspector.o: id3inspector.cpp ../common/values.h ../common/file_handling.h
	$(CXX) $(CXXFLAGS) -c $< -o $@
