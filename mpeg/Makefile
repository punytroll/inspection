CXXFLAGS = -std=c++17 -Wall -Werror -g -O0

default: all

all: common ../mpeginspector

clean:
	$(RM) ../mpeginspector
	$(RM) mpeginspector.o

common:
	$(MAKE) -C ../common

../mpeginspector: mpeginspector.o ../common/libcommon.a
	$(CXX) $(LDFLAGS) $^ -o $@

mpeginspector.o: mpeginspector.cpp ../common/include/common/*.h
	$(CXX) $(CXXFLAGS) -c $< -o $@ -I../common/include

.PHONY: common
