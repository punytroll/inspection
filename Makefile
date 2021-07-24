default: all

all:
	$(MAKE) -C libraries
	$(MAKE) -C common
	$(MAKE) -C inspectors
	$(MAKE) -C test

check: all
	$(MAKE) $@ -C libraries
	$(MAKE) $@ -C test

clean:
	$(MAKE) $@ -C libraries
	$(MAKE) $@ -C common
	$(MAKE) $@ -C inspectors
	$(MAKE) $@ -C test

.PHONY: all check clean default
