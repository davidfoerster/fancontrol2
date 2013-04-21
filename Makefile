SUBFOLDERS := Debug Optimized Release

PREFIX = /usr/local


.PHONY: all clean install


all: $(SUBFOLDERS)

clean:
	for sub in $(SUBFOLDERS); do \
		$(MAKE) -C "$$sub" clean; \
	done

install: Release
	install -d "$(PREFIX)/sbin"
	install -st "$(PREFIX)/sbin" Release/fancontrol2
	@echo "Successfully installed to \`$(PREFIX)'."


$(SUBFOLDERS):
	$(MAKE) -C "$@" all
