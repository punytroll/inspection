CXXFLAGS = -std=c++14 -Wall -Werror -g -O0
LDFLAGS = -g

default: all

clean:
	$(RM) ../vorbisinspector
	$(RM) vorbisinspector.o

all: ../vorbisinspector

../vorbisinspector: vorbisinspector.o
	$(CXX) $(LDFLAGS) $^ -o $@

vorbisinspector.o: vorbisinspector.cpp ../common/any_printing.h ../common/file_handling.h ../common/results.h
	$(CXX) $(CXXFLAGS) -c $< -o $@
