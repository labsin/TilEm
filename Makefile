all:
	cd emu && $(MAKE)
	cd db && $(MAKE)
	cd gui && $(MAKE)

clean:
	cd emu && $(MAKE) clean
	cd db && $(MAKE) clean
	cd gui && $(MAKE) clean

.PHONY: clean all
