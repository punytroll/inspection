CXXFLAGS = -std=c++14 -Wall -Werror -g -O0
LDFLAGS = -g

default: all

clean:
	$(RM) ../asfinspector
	$(RM) asfinspector.o
	$(RM) getters.o
	$(RM) helper.o

all: ../asfinspector

../asfinspector: asfinspector.o getters.o helper.o
	$(CXX) $(LDFLAGS) $^ -o $@

asfinspector.o: asfinspector.cpp ../common/any_printing.h ../common/file_handling.h ../common/guid.h ../common/5th/buffer.h ../common/5th/getters.h ../common/5th/length.h ../common/5th/result.h ../common/5th/value.h ../common/5th/value_printing.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

getters.o: ../common/5th/getters.cpp ../common/5th/buffer.h ../common/5th/getters.h ../common/5th/length.h ../common/5th/result.h ../common/5th/value.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

helper.o: ../common/helper.cpp ../common/helper.h
	$(CXX) $(CXXFLAGS) -c $< -o $@
