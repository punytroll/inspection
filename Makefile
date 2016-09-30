all:
	$(MAKE) -C id3
	$(MAKE) -C mpeg
	$(MAKE) -C riff
	$(MAKE) -C vorbis

clean:
	$(MAKE) -C id3 $@
	$(MAKE) -C mpeg $@
	$(MAKE) -C riff $@
	$(MAKE) -C vorbis $@
