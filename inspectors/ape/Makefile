root = ../..

include $(root)/Makefile.include

default: all

all: common $(root)/apeinspector

clean:
	$(RM) $(root)/apeinspector
	$(RM) apeinspector.o

common:
	$(MAKE) -C $(root)/common

$(root)/apeinspector: apeinspector.o $(root)/common/libcommon.a $(root)/build/subprojects/xml_parser/libxml_parser.a
	$(CXX) $(LDFLAGS) $^ -o $@

apeinspector.o: apeinspector.cpp $(root)/common/include/*.h
	$(CXX) $(CXXFLAGS) -c $< -o $@ -I$(root)/common/include

.PHONY: common
