default: all

all: getters/4th.o hexadecimal_strings.o

clean:
	$(RM) getters/4th.o
	$(RM) hexadecimal_strings.o

getters/4th.o: getters/4th.cpp getters/4th.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

hexadecimal_strings.o: hexadecimal_strings.cpp hexadecimal_strings.h
	$(CXX) $(CXXFLAGS) -c $< -o $@
