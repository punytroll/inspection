CXXFLAGS = -std=c++17 -Wall -Werror -g -O0
LDFLAGS = -g

default: all

all: common ../apeinspector

clean:
	$(RM) ../apeinspector
	$(RM) apeinspector.o

common:
	$(MAKE) -C ../common

../apeinspector: apeinspector.o ../common/libcommon.a
	$(CXX) $(LDFLAGS) $^ -o $@

apeinspector.o: apeinspector.cpp ../common/include/*.h
	$(CXX) $(CXXFLAGS) -c $< -o $@ -I../common/include

.PHONY: common
