CXXFLAGS = -std=c++14 -Wall -Werror -g -O0
LDFLAGS = -g

default: all

clean:
	$(RM) ../waveinspector
	$(RM) waveinspector.o

all: ../waveinspector

../waveinspector: waveinspector.o
	$(CXX) $(LDFLAGS) $^ -o $@

waveinspector.o: waveinspector.cpp ../common/file_handling.h ../common/values.h
	$(CXX) $(CXXFLAGS) -c $< -o $@
