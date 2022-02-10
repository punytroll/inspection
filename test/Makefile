include ../Makefile.include

default: all

all: common external test

check: all
	./test
	$(PYTHON) test.py --in=tests.xml

clean:
	$(MAKE) -C external clean
	$(RM) test
	$(RM) test.o
	$(RM) .test_statistics.csv

common:
	$(MAKE) -C ../common

external:
	$(MAKE) -C external

test: test.o ../common/libcommon.a ../libraries/xml_parser/build/libxml_parser.a
	$(CXX) $(LDFLAGS) $^ -o $@

test.o: test.cpp ../common/include/*.h
	$(CXX) $(CXXFLAGS) -c $< -o $@ -I../common/include

.PHONY: all common default external
