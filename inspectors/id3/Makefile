root = ../..

include $(root)/Makefile.include

default: all

all: common $(root)/id3inspector

clean:
	$(RM) $(root)/id3inspector
	$(RM) id3inspector.o

common:
	$(MAKE) -C $(root)/common

$(root)/id3inspector: id3inspector.o $(root)/common/libcommon.a $(root)/build/subprojects/xml_parser/libxml_parser.a
	$(CXX) $(LDFLAGS) $^ -o $@

id3inspector.o: id3inspector.cpp $(root)/common/include/*.h
	$(CXX) $(CXXFLAGS) -c $< -o $@ -I$(root)/common/include

.PHONY: common
