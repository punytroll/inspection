default: all

all:
	$(MAKE) -C ape
	$(MAKE) -C asf
	$(MAKE) -C flac
	$(MAKE) -C general
	$(MAKE) -C id3
	$(MAKE) -C mpeg
	$(MAKE) -C riff
	$(MAKE) -C vorbis

clean:
	$(MAKE) $@ -C ape
	$(MAKE) $@ -C asf
	$(MAKE) $@ -C flac
	$(MAKE) $@ -C general
	$(MAKE) $@ -C id3
	$(MAKE) $@ -C mpeg
	$(MAKE) $@ -C riff
	$(MAKE) $@ -C vorbis

.PHONY: all check clean default
