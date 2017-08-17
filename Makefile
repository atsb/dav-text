prefix=/usr/local
CC=gcc
OBJECTS=main.o \
        buffers.o \
        fileIO.o \
        screenIO.o \
        keyboard.o \
        features.o \
        undo.o \
        move.o
        
CFLAGS+=-O3 -Wall
LDFLAGS+=-lncurses -O3 -Wall

dav: $(OBJECTS) 
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) $(OBJECTS) -o dav

install:
	install -D dav $(DESTDIR)$(prefix)/bin/dav

uninstall:
	rm $(DESTDIR)$(prefix)/bin/dav

clean:
	-rm -rf dav core *.o tags
