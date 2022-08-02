root = ../..

include $(root)/Makefile.include

default: all

all: common $(root)/vorbisinspector

clean:
	$(RM) $(root)/vorbisinspector
	$(RM) vorbisinspector.o

common:
	$(MAKE) -C $(root)/common

$(root)/vorbisinspector: vorbisinspector.o $(root)/common/libcommon.a $(root)/build/subprojects/xml_parser/libxml_parser.a
	$(CXX) $(LDFLAGS) $^ -o $@

vorbisinspector.o: vorbisinspector.cpp $(root)/common/include/*.h
	$(CXX) $(CXXFLAGS) -c $< -o $@ -I$(root)/common/include

.PHONY: common
