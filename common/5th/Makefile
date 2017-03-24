CXXFLAGS = -std=c++14 -Wall -Werror -g -O0
LDFLAGS = -g

default: all

clean:
	$(RM) getters.o
	$(RM) helper.o
	$(RM) test
	$(RM) test.o

all: test getters.o

getters.o: getters.cpp getters.h ../guid.h result.h buffer.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

helper.o: ../helper.cpp ../helper.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

test: getters.o helper.o test.o
	$(CXX) $(LDFLAGS) $^ -o $@

test.o: test.cpp buffer.h length.h result.h value.h
	$(CXX) $(CXXFLAGS) -c $< -o $@
