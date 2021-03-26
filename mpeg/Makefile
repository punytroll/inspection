include ../Makefile.include

default: all

all: common ../mpeginspector

clean:
	$(RM) ../mpeginspector
	$(RM) mpeginspector.o

common:
	$(MAKE) -C ../common

../mpeginspector: mpeginspector.o ../common/libcommon.a
	$(CXX) $(LDFLAGS) $^ -o $@

mpeginspector.o: mpeginspector.cpp ../common/include/*.h
	$(CXX) $(CXXFLAGS) -c $< -o $@ -I../common/include

.PHONY: common
