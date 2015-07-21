.PHONY: all html latex install clean


all: html latex
	
html:
	doxygen clRNG.doxy

latex:
	$(MAKE) -C out/latex

install: html latex
	mkdir -p "$(PREFIX)/doc"
	install -d out/html "$(PREFIX)/doc"
	install --mode=0644 out/latex/refman.pdf "$(PREFIX)/doc"

clean:
	rm -rf out/html out/latex
