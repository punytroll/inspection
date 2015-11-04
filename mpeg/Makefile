CXXFLAGS = -std=c++14 -Wall -Werror -g -O0

default: all

clean:
	$(RM) ../mpeginspector
	$(RM) mpeginspector.o

all: ../mpeginspector

../mpeginspector: mpeginspector.o
	$(CXX) $(LDFLAGS) $^ -o $@

mpeginspector.o: mpeginspector.cpp ../common/file_handling.h
	$(CXX) $(CXXFLAGS) -c $< -o $@
