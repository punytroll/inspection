root = ../..

include $(root)/Makefile.include

default: all

all: common $(root)/generalinspector

clean:
	$(RM) $(root)/generalinspector
	$(RM) generalinspector.o

common:
	$(MAKE) -C $(root)/common

$(root)/generalinspector: generalinspector.o $(root)/common/libcommon.a $(root)/libraries/xml_parser/build/libxml_parser.a
	$(CXX) $(LDFLAGS) $^ -o $@

generalinspector.o: generalinspector.cpp $(root)/common/include/*.h
	$(CXX) $(CXXFLAGS) -c $< -o $@ -I$(root)/common/include

.PHONY: common
