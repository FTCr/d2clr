SHELL = /bin/sh 

NAME = d2clr

FNAME   = ${NAME}
OBJECTS = *.o
SOURCES = *.c

LC_M := LC_MESSAGES=C

PKG-CONFIG = pkg-config --cflags --libs

CLFLAGS = ${PKG-CONFIG} glib-2.0 dbus-1 dbus-glib-1 x11 xtst

.PHONY = all

all:
	${LC_M} gcc `${CLFLAGS}` -std=gnu99 ${SOURCES} -o ${FNAME}
