default: all

all: build
	meson compile -C build
	$(MAKE) -C common
	$(MAKE) -C inspectors
	$(MAKE) -C test

build:
	meson setup build

check: all
	meson test -C build
	$(MAKE) $@ -C test

clean:
	$(MAKE) $@ -C common
	$(MAKE) $@ -C inspectors
	$(MAKE) $@ -C test

.PHONY: all check clean default
