include ../Makefile.include

default: all

all: common ../id3inspector

clean:
	$(RM) ../id3inspector
	$(RM) id3inspector.o

common:
	$(MAKE) -C ../common

../id3inspector: id3inspector.o ../common/libcommon.a ../libraries/xml_parser/build/libxml_parser.a
	$(CXX) $(LDFLAGS) $^ -o $@

id3inspector.o: id3inspector.cpp ../common/include/*.h
	$(CXX) $(CXXFLAGS) -c $< -o $@ -I../common/include

.PHONY: common
