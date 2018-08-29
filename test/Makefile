CXXFLAGS = -std=c++17 -Wall -Werror -g -O0
LDFLAGS = -g

default: all

check:
	python3 test.py --in=tests.xml

clean:
	$(RM) ../test
	$(RM) test.o
	$(RM) getters.o
	$(RM) guid.o
	$(RM) helper.o
	$(RM) reader.o
	$(RM) string_cast.o

all: test

test: test.o getters.o guid.o helper.o reader.o string_cast.o
	$(CXX) $(LDFLAGS) $^ -o $@

test.o: test.cpp ../common/any_printing.h ../common/buffer.h ../common/file_handling.h ../common/getters.h ../common/length.h ../common/result.h ../common/value.h ../common/value_printing.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

getters.o: ../common/getters.cpp ../common/getters.h ../common/result.h ../common/value.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

guid.o: ../common/guid.cpp ../common/guid.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

helper.o: ../common/helper.cpp ../common/helper.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

reader.o: ../common/reader.cpp ../common/buffer.h ../common/length.h ../common/reader.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

string_cast.o: ../common/string_cast.cpp ../common/string_cast.h
	$(CXX) $(CXXFLAGS) -c $< -o $@
