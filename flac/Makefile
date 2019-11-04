CXXFLAGS = -std=c++17 -Wall -Werror -g -O0
LDFLAGS = -g

default: all

all: common ../flacinspector

clean:
	$(RM) ../flacinspector
	$(RM) flacinspector.o

common:
	$(MAKE) -C common

../flacinspector: flacinspector.o common/libcommon.a
	$(CXX) $(LDFLAGS) $^ -o $@ common/libcommon.a

flacinspector.o: flacinspector.cpp common/include/*.h
	$(CXX) $(CXXFLAGS) -c $< -o $@ -Icommon/include

.PHONY: common
