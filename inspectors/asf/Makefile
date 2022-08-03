root = ../..

include $(root)/Makefile.include

default: all

all: common $(root)/asfinspector

clean:
	$(RM) $(root)/asfinspector
	$(RM) asfinspector.o

common:
	$(MAKE) -C $(root)/common

$(root)/asfinspector: asfinspector.o $(root)/common/libcommon.a $(root)/build/subprojects/string_cast/libstring_cast.a $(root)/build/subprojects/xml_puny_dom/libxml_puny_dom.a $(root)/build/subprojects/xml_parser/libxml_parser.a
	$(CXX) $(LDFLAGS) $^ -o $@

asfinspector.o: asfinspector.cpp $(root)/common/include/*.h
	$(CXX) $(CXXFLAGS) -c $< -o $@ -I$(root)/common/include

.PHONY: common
