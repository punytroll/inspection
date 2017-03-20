CXXFLAGS = -std=c++14 -Wall -Werror -g -O0
LDFLAGS = -g

default: all

clean:
	$(RM) ../vorbisinspector
	$(RM) getters.o
	$(RM) vorbisinspector.o

all: ../vorbisinspector

../vorbisinspector: vorbisinspector.o getters.o
	$(CXX) $(LDFLAGS) $^ -o $@

getters.o: ../common/5th/getters.cpp ../common/5th/getters.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

vorbisinspector.o: vorbisinspector.cpp ../common/any_printing.h ../common/file_handling.h ../common/5th/getters.h ../common/5th/result.h ../common/unicode.h ../common/5th/value.h
	$(CXX) $(CXXFLAGS) -c $< -o $@
