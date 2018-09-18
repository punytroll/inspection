CXXFLAGS = -std=c++14 -Wall -Werror -g -O0
LDFLAGS = -g

default: all

all: common ../vorbisinspector

clean:
	$(RM) ../vorbisinspector
	$(RM) vorbisinspector.o

common:
	$(MAKE) -C common

../vorbisinspector: vorbisinspector.o common/libcommon.a
	$(CXX) $(LDFLAGS) $^ -o $@ common/libcommon.a

vorbisinspector.o: vorbisinspector.cpp common/include/*.h
	$(CXX) $(CXXFLAGS) -c $< -o $@ -Icommon/include

.PHONY: common
