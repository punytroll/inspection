CXXFLAGS = -std=c++17 -Wall -Werror -g -O0
LDFLAGS = -g

default: all

all: common ../asfinspector

clean:
	$(RM) ../asfinspector
	$(RM) asfinspector.o

common:
	$(MAKE) -C ../common

../asfinspector: asfinspector.o ../common/libcommon.a
	$(CXX) $(LDFLAGS) $^ -o $@

asfinspector.o: asfinspector.cpp ../common/include/common/*.h
	$(CXX) $(CXXFLAGS) -c $< -o $@ -I../common/include

.PHONY: common
