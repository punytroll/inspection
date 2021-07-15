include ../Makefile.include

default: all

all: common ../mpeginspector

clean:
	$(RM) ../mpeginspector
	$(RM) mpeginspector.o

common:
	$(MAKE) -C ../common

../mpeginspector: mpeginspector.o ../common/libcommon.a ../libraries/xml_parser/build/libxml_parser.a
	$(CXX) $(LDFLAGS) $^ -o $@

mpeginspector.o: mpeginspector.cpp ../common/include/*.h
	$(CXX) $(CXXFLAGS) -c $< -o $@ -I../common/include

.PHONY: common
