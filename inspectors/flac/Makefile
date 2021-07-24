root = ../..

include $(root)/Makefile.include

default: all

all: common $(root)/flacinspector

clean:
	$(RM) $(root)/flacinspector
	$(RM) flacinspector.o

common:
	$(MAKE) -C $(root)/common

$(root)/flacinspector: flacinspector.o $(root)/common/libcommon.a $(root)/libraries/xml_parser/build/libxml_parser.a
	$(CXX) $(LDFLAGS) $^ -o $@

flacinspector.o: flacinspector.cpp $(root)/common/include/*.h
	$(CXX) $(CXXFLAGS) -c $< -o $@ -I$(root)/common/include

.PHONY: common
