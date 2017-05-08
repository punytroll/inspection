CXXFLAGS = -std=c++14 -Wall -Werror -g -O0
LDFLAGS = -g

default: all

clean:
	$(RM) ../riffinspector
	$(RM) getters.o
	$(RM) guid.o
	$(RM) helper.o
	$(RM) riffinspector.o
	$(RM) string_cast.o

all: ../riffinspector

../riffinspector: riffinspector.o getters.o guid.o helper.o string_cast.o
	$(CXX) $(LDFLAGS) $^ -o $@

getters.o: ../common/getters.cpp ../common/getters.h ../common/length.h ../common/result.h ../common/value.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

guid.o: ../common/guid.cpp ../common/guid.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

helper.o: ../common/helper.cpp ../common/helper.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

string_cast.o: ../common/string_cast.cpp ../common/string_cast.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

riffinspector.o: riffinspector.cpp ../common/common.h ../common/file_handling.h ../common/getters.h ../common/length.h ../common/result.h ../common/value.h ../common/value_printing.h
	$(CXX) $(CXXFLAGS) -c $< -o $@
