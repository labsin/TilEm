all:
	cd core && $(MAKE)
	cd db && $(MAKE)
	cd gui && $(MAKE)

clean:
	cd core && $(MAKE) clean
	cd db && $(MAKE) clean
	cd gui && $(MAKE) clean

.PHONY: clean all
