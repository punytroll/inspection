CXXFLAGS = -std=c++14 -Wall -Werror -g -O0
LDFLAGS = -g

default: all

clean:
	$(RM) ../asfinspector
	$(RM) asfinspector.o

all: ../asfinspector

../asfinspector: asfinspector.o ../common/getters/4th.o ../common/hexadecimal_strings.o
	$(CXX) $(LDFLAGS) $^ -o $@

asfinspector.o: asfinspector.cpp ../common/any_printing.h ../common/file_handling.h ../common/getters/4th.h ../common/guid.h ../common/hexadecimal_strings.o ../common/results.h
	$(CXX) $(CXXFLAGS) -c $< -o $@
