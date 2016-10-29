default: all

all: getters/4th.o hexadecimal_strings.o

clean:
	$(RM) -f getters/4th.o
	$(RM) -f hexadecimal_strings.o

getters/4th.o: getters/4th.cpp getters/4th.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

hexadecimal_strings.o: hexadecimal_strings.cpp hexadecimal_strings.h
	$(CXX) $(CXXFLAGS) -c $< -o $@
