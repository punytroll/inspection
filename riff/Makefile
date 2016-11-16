CXXFLAGS = -std=c++14 -Wall -Werror -g -O0
LDFLAGS = -g

default: all

clean:
	$(RM) ../riffinspector
	$(RM) 4th.o
	$(RM) riffinspector.o
	$(RM) unicode.o

all: ../riffinspector

../riffinspector: riffinspector.o 4th.o unicode.o
	$(CXX) $(LDFLAGS) $^ -o $@

4th.o: ../common/getters/4th.cpp ../common/getters/4th.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

riffinspector.o: riffinspector.cpp ../common/file_handling.h ../common/getters/4th.h ../common/results.h ../common/unicode.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

unicode.o: ../common/unicode.cpp ../common/unicode.h
	$(CXX) $(CXXFLAGS) -c $< -o $@
