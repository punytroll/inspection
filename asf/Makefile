CXXFLAGS = -std=c++14 -Wall -Werror -g -O0
LDFLAGS = -g

default: all

clean:
	$(RM) ../asfinspector
	$(RM) 4th.o
	$(RM) asfinspector.o
	$(RM) hexadecimal_strings.o
	$(RM) unicode.o

all: ../asfinspector

../asfinspector: asfinspector.o 4th.o hexadecimal_strings.o unicode.o
	$(CXX) $(LDFLAGS) $^ -o $@

4th.o: ../common/getters/4th.cpp ../common/getters/4th.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

asfinspector.o: asfinspector.cpp ../common/any_printing.h ../common/file_handling.h ../common/getters/4th.h ../common/guid.h ../common/hexadecimal_strings.h ../common/unicode.h ../common/results.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

hexadecimal_strings.o: ../common/hexadecimal_strings.cpp ../common/hexadecimal_strings.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

unicode.o: ../common/unicode.cpp ../common/unicode.h
	$(CXX) $(CXXFLAGS) -c $< -o $@
