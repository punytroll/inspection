default: all

all: xml_parser

check: check-xml_parser

check-xml_parser: xml_parser/build
	meson test -C $<

clean: clean-xml_parser

clean-xml_parser:
	$(RM) -r xml_parser/build

xml_parser: xml_parser/build
	meson compile -C $<

xml_parser/build: xml_parser/meson.build
	mkdir $@ && \
	cd $@ && \
	meson setup -Ddefault_library=static

xml_parser/meson.build:
	git submodule update --init xml_parser

.PHONY: xml_parser check-xml_parser
