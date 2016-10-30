CXXFLAGS = -std=c++14 -Wall -Werror -g -O0
LDFLAGS = -g

default: all

clean:
	$(RM) ../riffinspector
	$(RM) riffinspector.o

all: ../riffinspector

../riffinspector: riffinspector.o ../common/getters/4th.o
	$(CXX) $(LDFLAGS) $^ -o $@

riffinspector.o: riffinspector.cpp ../common/file_handling.h ../common/getters/4th.h ../common/results.h
	$(CXX) $(CXXFLAGS) -c $< -o $@
