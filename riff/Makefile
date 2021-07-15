include ../Makefile.include

default: all

all: common ../riffinspector

clean:
	$(RM) ../riffinspector
	$(RM) riffinspector.o

common:
	$(MAKE) -C ../common

../riffinspector: riffinspector.o ../common/libcommon.a ../libraries/xml_parser/build/libxml_parser.a
	$(CXX) $(LDFLAGS) $^ -o $@

riffinspector.o: riffinspector.cpp ../common/include/*.h
	$(CXX) $(CXXFLAGS) -c $< -o $@ -I../common/include

.PHONY: common
