CXXFLAGS = -std=c++14 -Wall -Werror -g -O0
LDFLAGS = -g

default: all

clean:
	$(RM) ../riffinspector
	$(RM) riffinspector.o

all: ../riffinspector

../riffinspector: riffinspector.o
	$(CXX) $(LDFLAGS) $^ -o $@

riffinspector.o: riffinspector.cpp ../common/file_handling.h ../common/values.h
	$(CXX) $(CXXFLAGS) -c $< -o $@
