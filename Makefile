PREFIX ?= /usr/local
BINDIR := $(PREFIX)/bin
MANDIR := $(PREFIX)/share/man/man1
MANPAGE := man/man1/sdltimer.1

CFLAGS := -Wall -Wextra $(shell pkg-config --cflags sdl3)
LIBS := $(shell pkg-config --libs sdl3)
OBJDIR := build

.PHONY: default clean install uninstall

default: sdltimer

$(OBJDIR):
	mkdir -p $(OBJDIR)

sdltimer: $(OBJDIR)/main.o $(OBJDIR)/timer.o
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

$(OBJDIR)/main.o: main.c timer.h config.h | $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJDIR)/timer.o: timer.c timer.h | $(OBJDIR)
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
