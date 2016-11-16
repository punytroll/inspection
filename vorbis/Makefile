CXXFLAGS = -std=c++14 -Wall -Werror -g -O0
LDFLAGS = -g

default: all

clean:
	$(RM) ../vorbisinspector
	$(RM) 4th.o
	$(RM) vorbisinspector.o
	$(RM) unicode.o

all: ../vorbisinspector

../vorbisinspector: vorbisinspector.o 4th.o unicode.o
	$(CXX) $(LDFLAGS) $^ -o $@

4th.o: ../common/getters/4th.cpp ../common/getters/4th.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

vorbisinspector.o: vorbisinspector.cpp ../common/any_printing.h ../common/file_handling.h ../common/getters/4th.h ../common/results.h ../common/unicode.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

unicode.o: ../common/unicode.cpp ../common/unicode.h
	$(CXX) $(CXXFLAGS) -c $< -o $@
