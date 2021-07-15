include ../Makefile.include

default: all

all: common test

check: all
	./test
	$(PYTHON) test.py --in=tests.xml

clean:
	$(RM) test
	$(RM) test.o
	$(RM) .test_statistics.csv

common:
	$(MAKE) -C ../common

test: test.o ../common/libcommon.a ../libraries/xml_parser/build/libxml_parser.a
	$(CXX) $(LDFLAGS) $^ -o $@

test.o: test.cpp ../common/include/*.h
	$(CXX) $(CXXFLAGS) -c $< -o $@ -I../common/include

.PHONY: all common default
