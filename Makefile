SUBFOLDERS := Debug Optimized Release
EXE = fancontrol2
PREFIX = /usr/local


.PHONY: all clean install


all: $(SUBFOLDERS)

clean:
	for sub in $(SUBFOLDERS); do \
		$(MAKE) -C "$$sub" clean; \
	done

install: Release/$(EXE)
	install -d "$(PREFIX)/sbin"
	install -st "$(PREFIX)/sbin" $<
	@echo "Successfully installed to \`$(PREFIX)'."

$(SUBFOLDERS):
	$(MAKE) -C "$@" all
