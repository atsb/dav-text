CC=gcc
OBJECTS=main.o \
        buffers.o \
        fileIO.o \
        screenIO.o \
        keyboard.o \
        features.o \
        undo.o \
        move.o
        
CFLAGS=-O3 -Wall
LDFLAGS=-lncurses -O3 -Wall

dav: $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o dav

install:
	mkdir -p $(DESTDIR)/usr/bin
	mkdir -p $(DESTDIR)/usr/share/man/man1
	cp dav $(DESTDIR)/usr/bin/dav
	cp dav.1.gz $(DESTDIR)/usr/share/man/man1/

uninstall:
	rm $(DESTDIR)/usr/bin/dav

clean:
	-rm -rf dav core *.o tags
