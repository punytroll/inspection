all:
	$(MAKE) -C id3
	$(MAKE) -C mpeg
	$(MAKE) -C wave

clean:
	$(MAKE) -C id3 $@
	$(MAKE) -C mpeg $@
	$(MAKE) -C wave $@
