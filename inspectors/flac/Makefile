root = ../..

include $(root)/Makefile.include

default: all

all: common $(root)/flacinspector

clean:
	$(RM) $(root)/flacinspector
	$(RM) flacinspector.o

common:
	$(MAKE) -C $(root)/common

$(root)/flacinspector: flacinspector.o $(root)/common/libcommon.a $(root)/build/subprojects/string_cast/libstring_cast.a $(root)/build/subprojects/xml_puny_dom/libxml_puny_dom.a $(root)/build/subprojects/xml_parser/libxml_parser.a
	$(CXX) $(LDFLAGS) $^ -o $@

flacinspector.o: flacinspector.cpp $(root)/common/include/*.h
	$(CXX) $(CXXFLAGS) -c $< -o $@ -I$(root)/common/include

.PHONY: common
