CXXFLAGS = -std=c++14 -Wall -Werror -g -O0
LDFLAGS = -g

default: all

clean:
	$(RM) getters.o
	$(RM) guid.o
	$(RM) helper.o
	$(RM) string_cast.o
	$(RM) test
	$(RM) test.o

all: test

getters.o: getters.cpp getters.h guid.h result.h buffer.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

guid.o: guid.cpp guid.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

helper.o: helper.cpp helper.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

string_cast.o: string_cast.cpp string_cast.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

test: getters.o guid.o helper.o string_cast.o test.o
	$(CXX) $(LDFLAGS) $^ -o $@

test.o: test.cpp buffer.h length.h result.h value.h
	$(CXX) $(CXXFLAGS) -c $< -o $@
