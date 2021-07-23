root = ../..

include $(root)/Makefile.include

default: all

all: common $(root)/riffinspector

clean:
	$(RM) $(root)/riffinspector
	$(RM) riffinspector.o

common:
	$(MAKE) -C $(root)/common

$(root)/riffinspector: riffinspector.o $(root)/common/libcommon.a $(root)/libraries/xml_parser/build/libxml_parser.a
	$(CXX) $(LDFLAGS) $^ -o $@

riffinspector.o: riffinspector.cpp $(root)/common/include/*.h
	$(CXX) $(CXXFLAGS) -c $< -o $@ -I$(root)/common/include

.PHONY: common
