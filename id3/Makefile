CXXFLAGS = -std=c++14 -Wall -Werror -g -O0

default: all

clean:
	$(RM) ../id3inspector
	$(RM) getters.o
	$(RM) guid.o
	$(RM) helper.o
	$(RM) id3inspector.o
	$(RM) string_cast.o

all: ../id3inspector

../id3inspector: getters.o guid.o helper.o id3inspector.o string_cast.o
	$(CXX) $(LDFLAGS) $^ -o $@

getters.o: ../common/5th/getters.cpp ../common/5th/getters.h ../common/5th/not_implemented_exception.h ../common/5th/result.h ../common/5th/value.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

helper.o: ../common/helper.cpp ../common/helper.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

guid.o: ../common/guid.cpp ../common/guid.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

id3inspector.o: id3inspector.cpp ../common/any_printing.h ../common/values.h ../common/file_handling.h ../common/5th/getters.h ../common/5th/value_printing.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

string_cast.o: ../common/string_cast.cpp ../common/string_cast.h
	$(CXX) $(CXXFLAGS) -c $< -o $@
