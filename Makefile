default: all

all:
	$(MAKE) -C common
	$(MAKE) -C asf
	$(MAKE) -C flac
	$(MAKE) -C id3
	$(MAKE) -C mpeg
	$(MAKE) -C riff
	$(MAKE) -C vorbis

clean:
	$(MAKE) -C common $@
	$(MAKE) -C asf $@
	$(MAKE) -C flac
	$(MAKE) -C id3 $@
	$(MAKE) -C mpeg $@
	$(MAKE) -C riff $@
	$(MAKE) -C vorbis $@
