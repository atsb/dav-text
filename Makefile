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
LDFLAGS+=-lncurses -z now

dav: $(OBJECTS) 
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) $(OBJECTS) -o dav

install:
	install -D dav $(DESTDIR)$(prefix)/bin/dav
<<<<<<< HEAD
	mkdir -p $(DESTDIR)$(prefix)/share/man/man1
	install -D dav.1.gz $(DESTDIR)$(prefix)/share/man/man1/
=======
	install -D dav.1.gz $(DESTDIR)$(prefix)/share/man/man1
>>>>>>> 90a4a6aa050a88ce9ad3675a4e58f751c298f38d

uninstall:
	rm $(DESTDIR)$(prefix)/bin/dav
	rm $(DESTDIR)$(prefix)/share/man/man1/dav.1.gz

clean:
	-rm -rf dav core *.o tags