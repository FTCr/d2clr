SHELL = /bin/sh 

NAME = d2clrd

OBJECTS = *.o
SOURCES = *.c

LC_M := LC_MESSAGES=C

PKG-CONFIG = pkg-config --cflags --libs

CLFLAGS = ${PKG-CONFIG} glib-2.0 dbus-1 dbus-glib-1 x11 xtst

.PHONY = all

ifdef PREFIX
	prefix=${PREFIX}
else
	prefix=/usr/local
endif

all:
	${LC_M} gcc -std=gnu99 ${SOURCES} -o ${NAME} `${CLFLAGS}`
install:
	${LC_M} gcc -std=gnu99 ${SOURCES} -o ${NAME} `${CLFLAGS}`
	install -d "${prefix}/bin"
	install -s ${NAME} "${prefix}/bin"
uninstall:
	rm "${prefix}/bin/${NAME}"
clean:
	rm "${NAME}"
