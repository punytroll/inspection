root = ../..

include $(root)/Makefile.include

default: all

all: common $(root)/mpeginspector

clean:
	$(RM) $(root)/mpeginspector
	$(RM) mpeginspector.o

common:
	$(MAKE) -C $(root)/common

$(root)/mpeginspector: mpeginspector.o $(root)/common/libcommon.a $(root)/libraries/xml_parser/build/libxml_parser.a
	$(CXX) $(LDFLAGS) $^ -o $@

mpeginspector.o: mpeginspector.cpp $(root)/common/include/*.h
	$(CXX) $(CXXFLAGS) -c $< -o $@ -I$(root)/common/include

.PHONY: common
