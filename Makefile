default: all

clean:
	$(RM) id3inspector
	$(RM) id3inspector.o

all: id3inspector

id3inspector: id3inspector.o
	$(CXX) $(LDFLAGS) $^ -o $@

id3inspector.o: id3inspector.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
