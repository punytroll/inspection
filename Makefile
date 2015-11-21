all:
	$(MAKE) -C id3
	$(MAKE) -C mpeg
	$(MAKE) -C riff

clean:
	$(MAKE) -C id3 $@
	$(MAKE) -C mpeg $@
	$(MAKE) -C riff $@
