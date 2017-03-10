CXXFLAGS = -std=c++14 -Wall -Werror -g -O0
LDFLAGS = -g

default: all

clean:
	$(RM) ../flacinspector
	$(RM) flacinspector.o

all: ../flacinspector

../flacinspector: flacinspector.o
	$(CXX) $(LDFLAGS) $^ -o $@

flacinspector.o: flacinspector.cpp ../common/any_printing.h ../common/file_handling.h ../common/5th/buffer.h ../common/5th/getters.h ../common/5th/length.h ../common/5th/result.h ../common/5th/value.h
	$(CXX) $(CXXFLAGS) -c $< -o $@
