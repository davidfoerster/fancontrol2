SUBFOLDERS := Debug Optimized Profile Release
EXE = fancontrol2
PREFIX = /usr/local


.PHONY: all clean install


all: Release

clean:
	for sub in $(SUBFOLDERS); do \
		[ ! -d "$$sub" ] || $(MAKE) -C "$$sub" "$@"; \
	done

install: Release/src/$(EXE) | Release
	install -Dst '$(PREFIX)/sbin' -- '$<'
	@echo "Successfully installed to \`$(PREFIX)'."

$(SUBFOLDERS):
	$(MAKE) -C '$@' all

#Release/$(EXE): Profile/src/$(EXE).gcda
#	$(MAKE) -C Release $(EXE)

#Profile/src/$(EXE).gcda: Profile/$(EXE)
#	$(MAKE) -C Profile $(EXE)
#	$< $(EXE).yaml || { find Profile -name \*.gcda -delete; false; }
