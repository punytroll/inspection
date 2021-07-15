default: all

all: xml_parser

check: check-xml_parser

check-xml_parser: xml_parser/build
	cd $< && \
	ctest

clean: clean-xml_parser

clean-xml_parser:
	$(RM) -r xml_parser/build

xml_parser: xml_parser/build
	cmake --build $<

xml_parser/build:
	mkdir $@ && \
	cd $@ && \
	cmake ..

.PHONY: xml_parser check-xml_parser
