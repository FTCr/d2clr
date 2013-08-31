SHELL = /bin/sh 

NAME = d2clr

FNAME   = ${NAME}
OBJECTS = *.o
SOURCES = *.c

LC_M := LC_MESSAGES=C

PKG-CONFIG = pkg-config --cflags --libs

CLFLAGS = ${PKG-CONFIG} glib-2.0 dbus-1 dbus-glib-1 x11 xtst

.PHONY = all

ifdef EXEC_PREFIX
	exec_prefix=${EXEC_PREFIX}
else
	exec_prefix=/usr/local
endif


all:
	${LC_M} gcc `${CLFLAGS}` -std=gnu99 ${SOURCES} -o ${FNAME}
install:
	${LC_M} gcc `${CLFLAGS}` -std=gnu99 ${SOURCES} -o ${FNAME}
	install -d "${exec_prefix}/bin"
	install -s ${NAME} "${exec_prefix}/bin"
uninstall:
	rm "${exec_prefix}/bin/${FNAME}"
clean:
	rm "${FNAME}"