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
        
CFLAGS+=-O3 -D_FORTIFY_SOURCE=2 -O1 -g -Wall -Wextra -Wpointer-arith -Wuninitialized -Wshadow -Winit-self -Wmissing-declarations -fomit-frame-pointer -Wformat -Wformat-security -Werror=format-security
LDFLAGS+=-lncurses

dav: $(OBJECTS) 
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) $(OBJECTS) -o dav

install:
	install -D dav $(DESTDIR)$(prefix)/bin/dav

uninstall:
	rm $(DESTDIR)$(prefix)/bin/dav

clean:
	-rm -rf dav core *.o tags
