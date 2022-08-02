root = ../..

include $(root)/Makefile.include

default: all

all: common $(root)/mpeginspector

clean:
	$(RM) $(root)/mpeginspector
	$(RM) mpeginspector.o

common:
	$(MAKE) -C $(root)/common

$(root)/mpeginspector: mpeginspector.o $(root)/common/libcommon.a $(root)/build/subprojects/string_cast/libstring_cast.a $(root)/build/subprojects/xml_parser/libxml_parser.a
	$(CXX) $(LDFLAGS) $^ -o $@

mpeginspector.o: mpeginspector.cpp $(root)/common/include/*.h
	$(CXX) $(CXXFLAGS) -c $< -o $@ -I$(root)/common/include

.PHONY: common
