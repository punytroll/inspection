include ../Makefile.include

default: all

all: common ../generalinspector

clean:
	$(RM) ../generalinspector
	$(RM) generalinspector.o

common:
	$(MAKE) -C ../common

../generalinspector: generalinspector.o ../common/libcommon.a ../libraries/xml_parser/build/libxml_parser.a
	$(CXX) $(LDFLAGS) $^ -o $@

generalinspector.o: generalinspector.cpp ../common/include/*.h
	$(CXX) $(CXXFLAGS) -c $< -o $@ -I../common/include

.PHONY: common
