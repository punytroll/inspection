CXXFLAGS = -std=c++14 -Wall -Werror -g -O0
LDFLAGS = -g

default: all

clean:
	$(RM) ../riffinspector
	$(RM) getters.o
	$(RM) helper.o
	$(RM) riffinspector.o

all: ../riffinspector

../riffinspector: riffinspector.o getters.o helper.o
	$(CXX) $(LDFLAGS) $^ -o $@

getters.o: ../common/5th/getters.cpp ../common/5th/length.h ../common/5th/getters.h ../common/5th/length.h ../common/5th/result.h ../common/5th/value.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

helper.o: ../common/helper.cpp ../common/helper.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

riffinspector.o: riffinspector.cpp ../common/file_handling.h ../common/5th.h ../common/5th/length.h ../common/5th/getters.h ../common/5th/length.h ../common/5th/result.h ../common/5th/value.h
	$(CXX) $(CXXFLAGS) -c $< -o $@
