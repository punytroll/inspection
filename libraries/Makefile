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

xml_parser/build: xml_parser/CMakeLists.txt
	mkdir $@ && \
	cd $@ && \
	cmake ..

xml_parser/CMakeLists.txt:
	git submodule update --init xml_parser

.PHONY: xml_parser check-xml_parser
