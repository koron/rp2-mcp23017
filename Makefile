PROGRAM_NAME = mcp23017

default: build/$(PROGRAM_NAME).uf2

build/Makefile:
	cmake -B build -DCMAKE_DEPENDS_USE_COMPILER=OFF

.PHONY: build/$(PROGRAM_NAME).uf2
build/$(PROGRAM_NAME).uf2: build/Makefile
	make -j8 -C build

.PHONY: clean
clean:
	if [ -f build/Makefile ] ; then make -C build clean ; fi

.PHONY: distclean
distclean:
	rm -rf build
	rm -f tags

.PHONY: tags
tags:
	ctags --exclude=build/ -R .
