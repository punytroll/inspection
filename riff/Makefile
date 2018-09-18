CXXFLAGS = -std=c++14 -Wall -Werror -g -O0
LDFLAGS = -g

default: all

all: ../riffinspector

clean:
	$(RM) ../riffinspector
	$(RM) riffinspector.o

common:
	$(MAKE) -C common

../riffinspector: riffinspector.o common/libcommon.a
	$(CXX) $(LDFLAGS) $^ -o $@ common/libcommon.a

riffinspector.o: riffinspector.cpp common/include/*.h
	$(CXX) $(CXXFLAGS) -c $< -o $@ -Icommon/include

.PHONY: common
