PREFIX ?= /usr/local
BINDIR := $(PREFIX)/bin
MANDIR := $(PREFIX)/share/man/man1
MANPAGE := man/man1/sdltimer.1

CFLAGS := -Wall -Wextra -Iinclude $(shell pkg-config --cflags sdl3)
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
	mkdir -p $(BINDIR) $(MANDIR)
	cp sdltimer $(BINDIR)/sdltimer
	cp $(MANPAGE) $(MANDIR)/sdltimer.1
	chmod 755 $(BINDIR)/sdltimer

uninstall:
	rm -f $(BINDIR)/sdltimer $(MANDIR)/sdltimer.1

clean:
	rm -rf sdltimer $(OBJDIR)
