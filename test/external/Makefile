default: all

all: bmpsuite

bmpsuite: bmpsuite/bmpsuite

bmpsuite/bmpsuite: bmpsuite/Makefile
	$(MAKE) -C bmpsuite

bmpsuite/Makefile:
	git submodule update --init bmpsuite

clean:
	$(MAKE) -C bmpsuite clean

.PHONY: all clean
