default: all

clean:
	$(RM) ../mpeginspector
	$(RM) mpeginspector.o

all: ../mpeginspector

../mpeginspector: mpeginspector.o
	$(CXX) $(LDFLAGS) $^ -o $@

mpeginspector.o: mpeginspector.cpp
	$(CXX) $(CXXFLAGS) -std=c++0x -c $< -o $@
