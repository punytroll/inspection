CXXFLAGS = -std=c++17 -Wall -Werror -g -O0
LDFLAGS = -g

default: all

all: common test

check: all
	./test
	python3 test.py --in=tests.xml

clean:
	$(RM) test
	$(RM) test.o

common:
	$(MAKE) -C ../common

test: test.o ../common/libcommon.a
	$(CXX) $(LDFLAGS) $^ -o $@

test.o: test.cpp ../common/include/*.h
	$(CXX) $(CXXFLAGS) -c $< -o $@ -I../common/include

.PHONY: all common default
