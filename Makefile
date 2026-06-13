PREFIX ?= /usr/local
BINDIR := $(PREFIX)/bin
MANDIR := $(PREFIX)/share/man/man1
MANPAGE := man/man1/sdltimer.1

GIT_HASH := $(shell git rev-parse HEAD 2>/dev/null || echo unknown)

CFLAGS := -Wall -Wextra -Iinclude $(shell pkg-config --cflags sdl3) -DGIT_HASH=\"$(GIT_HASH)\"
LIBS := $(shell pkg-config --libs sdl3)
OBJDIR := build

.PHONY: default clean install uninstall help

default: sdltimer

help:
	@echo "Usage: make <Target>"
	@echo
	@echo "Targets:"
	@echo "  make            - Build sdltimer binary"
	@echo "  make install    - Install to $(PREFIX)"
	@echo "  make uninstall  - Remove installed files"
	@echo "  make reinstall  - Reinstall installed files"
	@echo "  make clean      - Remove build artifacts"
	@echo "  make help       - Show this message"

$(OBJDIR):
	mkdir -p $(OBJDIR)

sdltimer: $(OBJDIR)/main.o $(OBJDIR)/timer.o
	 $(CC) $(CFLAGS) -o $@ $^ $(LIBS)

$(OBJDIR)/main.o: main.c include/timer.h include/config.h | $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJDIR)/timer.o: timer.c include/timer.h | $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

install: sdltimer
	sudo mkdir -p $(BINDIR) $(MANDIR)
	sudo cp sdltimer $(BINDIR)/sdltimer
	sudo cp $(MANPAGE) $(MANDIR)/sdltimer.1
	sudo chmod 755 $(BINDIR)/sdltimer

uninstall:
	sudo rm -f $(BINDIR)/sdltimer $(MANDIR)/sdltimer.1

reinstall: uninstall install

clean:
	rm -rf sdltimer $(OBJDIR)
