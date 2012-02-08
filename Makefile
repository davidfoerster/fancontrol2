SUBFOLDERS := Debug Optimized Release

.PHONY: $(SUBFOLDERS) all clean install

all: $(SUBFOLDERS)

clean:
	for sub in $(SUBFOLDERS); do \
		$(MAKE) -C "$$sub" clean; \
	done

install: Release
	install -st /usr/local/sbin Release/fancontrol2

$(SUBFOLDERS):
	$(MAKE) -C "$@" all
