CXXFLAGS = -std=c++14 -Wall -Werror -g -O0
LDFLAGS = -g

default: all

all: common ../generalinspector

clean:
	$(RM) ../generalinspector
	$(RM) generalinspector.o

common:
	$(MAKE) -C common

../generalinspector: generalinspector.o common/libcommon.a
	$(CXX) $(LDFLAGS) $^ -o $@ common/libcommon.a

generalinspector.o: generalinspector.cpp common/include/*.h
	$(CXX) $(CXXFLAGS) -c $< -o $@ -Icommon/include

.PHONY: common
