SUBFOLDERS := Debug Optimized Release

PREFIX = /usr/local


.PHONY: $(SUBFOLDERS) all clean install


all: $(SUBFOLDERS)

clean:
	for sub in $(SUBFOLDERS); do \
		$(MAKE) -C "$$sub" clean; \
	done

install: Release
	install -st "$(PREFIX)/sbin" Release/fancontrol2
	@echo "Successfully installed to \`$(PREFIX)'."


$(SUBFOLDERS):
	$(MAKE) -C "$@" all
