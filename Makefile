default: all

all:
	$(MAKE) -C common
	$(MAKE) -C ape
	$(MAKE) -C asf
	$(MAKE) -C flac
	$(MAKE) -C general
	$(MAKE) -C id3
	$(MAKE) -C mpeg
	$(MAKE) -C riff
	$(MAKE) -C test
	$(MAKE) -C vorbis

clean:
	$(MAKE) $@ -C common
	$(MAKE) $@ -C ape
	$(MAKE) $@ -C asf
	$(MAKE) $@ -C flac
	$(MAKE) $@ -C general
	$(MAKE) $@ -C id3
	$(MAKE) $@ -C mpeg
	$(MAKE) $@ -C riff
	$(MAKE) $@ -C test
	$(MAKE) $@ -C vorbis
