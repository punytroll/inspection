CXXFLAGS = -std=c++14 -Wall -Werror -g -O0
LDFLAGS = -g

default: all

clean:
	$(RM) ../asfinspector
	$(RM) asfinspector.o

all: ../asfinspector

../asfinspector: asfinspector.o ../common/getters/4th.o
	$(CXX) $(LDFLAGS) $^ -o $@

asfinspector.o: asfinspector.cpp ../common/any_printing.h ../common/file_handling.h ../common/results.h ../common/getters/4th.h
	$(CXX) $(CXXFLAGS) -c $< -o $@
