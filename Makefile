SUBFOLDERS := Debug Optimized Release

.PHONY: $(SUBFOLDERS) all clean install

all: $(SUBFOLDERS)

clean:
	$(foreach sub, $(SUBFOLDERS), $(MAKE) -C $(sub) clean)

install: Release
	install -st /usr/local/sbin Release/fancontrol2

$(SUBFOLDERS):
	$(MAKE) -C "$@" all