# exas version
VERSION = 0.1.0

# Customize below to fit your system

# paths
PREFIX = /usr/local
MANPREFIX = ${PREFIX}/share/man
LIBS = -lm -lcrypt

# flags
CPPFLAGS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_XOPEN_SOURCE=700L -DVERSION=\"${VERSION}\"
CFLAGS = -std=c17 -pedantic -Wall ${CPPFLAGS}
LDFLAGS = ${LIBS}

# compiler (and linker)
CC = clang
