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
        
CFLAGS+=-D_FORTIFY_SOURCE=2 -g -Wall -Wextra -Wpointer-arith -Wuninitialized -Wshadow -Winit-self -Wmissing-declarations -Wformat -Wformat-security -Werror=format-security
LDFLAGS+=-lncurses -z now

dav: $(OBJECTS) 
	$(CC) $(CFLAGS) $(CPPFLAGS) $(OBJECTS) $(LDFLAGS) -o dav

install:
	install -D dav $(DESTDIR)$(prefix)/bin/dav
	mkdir -p $(DESTDIR)$(prefix)/share/man/man1
	install -D dav.1.gz $(DESTDIR)$(prefix)/share/man/man1/

uninstall:
	rm $(DESTDIR)$(prefix)/bin/dav
	rm $(DESTDIR)$(prefix)/share/man/man1/dav.1.gz

clean:
	-rm -rf dav core *.o tags
