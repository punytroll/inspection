include ../Makefile.include

default: all

all: common ../asfinspector

clean:
	$(RM) ../asfinspector
	$(RM) asfinspector.o

common:
	$(MAKE) -C ../common

../asfinspector: asfinspector.o ../common/libcommon.a ../libraries/xml_parser/build/libxml_parser.a
	$(CXX) $(LDFLAGS) $^ -o $@

asfinspector.o: asfinspector.cpp ../common/include/*.h
	$(CXX) $(CXXFLAGS) -c $< -o $@ -I../common/include

.PHONY: common
