all:
	cd core && $(MAKE)
	cd gui && $(MAKE)

clean:
	cd core && $(MAKE) clean
	cd gui && $(MAKE) clean

.PHONY: clean all
