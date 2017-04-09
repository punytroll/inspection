CXXFLAGS = -std=c++14 -Wall -Werror -g -O0
LDFLAGS = -g

default: all

clean:
	$(RM) ../vorbisinspector
	$(RM) getters.o
	$(RM) guid.o
	$(RM) helper.o
	$(RM) string_cast.o
	$(RM) vorbisinspector.o

all: ../vorbisinspector

../vorbisinspector: vorbisinspector.o getters.o guid.o helper.o string_cast.o
	$(CXX) $(LDFLAGS) $^ -o $@

getters.o: ../common/5th/getters.cpp ../common/5th/getters.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

guid.o: ../common/guid.cpp ../common/guid.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

helper.o: ../common/helper.cpp ../common/helper.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

string_cast.o: ../common/string_cast.cpp ../common/string_cast.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

vorbisinspector.o: vorbisinspector.cpp ../common/any_printing.h ../common/file_handling.h ../common/5th/buffer.h ../common/5th/getters.h ../common/5th/length.h ../common/5th/result.h ../common/5th/value.h ../common/5th/value_printing.h
	$(CXX) $(CXXFLAGS) -c $< -o $@
