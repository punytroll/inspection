CXXFLAGS = -std=c++17 -Wall -Werror -g -O0

default: all

all: ../mpeginspector

clean:
	$(RM) ../mpeginspector
	$(RM) mpeginspector.o

common:
	$(MAKE) -C common

../mpeginspector: mpeginspector.o common/libcommon.a
	$(CXX) $(LDFLAGS) $^ -o $@ common/libcommon.a

mpeginspector.o: mpeginspector.cpp common/include/*.h
	$(CXX) $(CXXFLAGS) -c $< -o $@ -Icommon/include

.PHONY: common
