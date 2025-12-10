# Makefile for shm_rw programs
# Supports both SE mode (bare metal) and FS mode (Linux) versions

CC = gcc
CFLAGS = -Wall -O2 -static
CFLAGS_FS = -Wall -O2

# Targets
TARGETS = shm_rw shm_rw_fs

all: $(TARGETS)

# SE mode version - static binary for gem5 SE mode
shm_rw: shm_rw.c
	$(CC) $(CFLAGS) -o $@ $<

# FS mode version - static binary for Linux (same as SE mode)
shm_rw_fs: shm_rw_fs.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGETS)

# Help
help:
	@echo "Available targets:"
	@echo "  all        - Build both SE and FS mode versions"
	@echo "  shm_rw     - Build SE mode version (static binary)"
	@echo "  shm_rw_fs  - Build FS mode version (Linux binary)"
	@echo "  clean      - Remove built binaries"
	@echo ""
	@echo "Usage:"
	@echo "  SE mode:  ./build/X86/gem5.opt configs/learning_gem5/part1/shm.py"
	@echo "  FS mode:  ./build/X86/gem5.opt configs/learning_gem5/part1/shm_fs.py"

.PHONY: all clean help
