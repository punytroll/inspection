include ../Makefile.include

default: all

all: common ../vorbisinspector

clean:
	$(RM) ../vorbisinspector
	$(RM) vorbisinspector.o

common:
	$(MAKE) -C ../common

../vorbisinspector: vorbisinspector.o ../common/libcommon.a
	$(CXX) $(LDFLAGS) $^ -o $@

vorbisinspector.o: vorbisinspector.cpp ../common/include/*.h
	$(CXX) $(CXXFLAGS) -c $< -o $@ -I../common/include

.PHONY: common
