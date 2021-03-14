CXXFLAGS = -std=c++17 -Wall -Werror -g -O0
LDFLAGS = -g

default: all

all: common ../vorbisinspector

clean:
	$(RM) ../vorbisinspector
	$(RM) vorbisinspector.o

common:
	$(MAKE) -C ../common

../vorbisinspector: vorbisinspector.o ../common/libcommon.a
	$(CXX) $(LDFLAGS) $^ -o $@

vorbisinspector.o: vorbisinspector.cpp ../common/include/common/*.h
	$(CXX) $(CXXFLAGS) -c $< -o $@ -I../common/include

.PHONY: common
