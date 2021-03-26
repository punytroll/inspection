include ../Makefile.include

default: all

all: common ../flacinspector

clean:
	$(RM) ../flacinspector
	$(RM) flacinspector.o

common:
	$(MAKE) -C ../common

../flacinspector: flacinspector.o ../common/libcommon.a
	$(CXX) $(LDFLAGS) $^ -o $@

flacinspector.o: flacinspector.cpp ../common/include/*.h
	$(CXX) $(CXXFLAGS) -c $< -o $@ -I../common/include

.PHONY: common
