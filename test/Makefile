root = ..

include $(root)/Makefile.include

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
	$(MAKE) -C $(root)/common

external:
	$(MAKE) -C external

test: test.o $(root)/common/libcommon.a $(root)/build/subprojects/string_cast/libstring_cast.a $(root)/build/subprojects/xml_puny_dom/libxml_puny_dom.a $(root)/build/subprojects/xml_parser/libxml_parser.a
	$(CXX) $(LDFLAGS) $^ -o $@

test.o: test.cpp $(root)/common/include/*.h
	$(CXX) $(CXXFLAGS) -c $< -o $@ -I$(root)/common/include

.PHONY: all common default external
