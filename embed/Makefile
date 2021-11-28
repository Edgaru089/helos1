

EMBED_DIR   := $(dir $(lastword $(MAKEFILE_LIST)))
initrd_files = $(shell find $(EMBED_DIR)initrd -type f)

initrd: $(initrd_files)
	echo "  TAR   $(EMBED_DIR)initrd.tar"
	cd $(EMBED_DIR)initrd && tar -cf ../initrd.tar -H ustar *

