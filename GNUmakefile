# exas - execute as
# See LICENSE file for copyright and license details

include config.mk

SRC = exas.c
OBJ = ${SRC:.c=.o}

all: options exas

options:
	@echo exas build options
	@echo "LDFLAGS= ${LDFLAGS}"
	@echo "CFLAGS=  ${CFLAGS}"
	@echo "CC=      ${CC}"

.c.o:
	${CC} -c ${CFLAGS} $<

${OBJ}: config.h config.mk

config.h:
	cp config.def.h $@

exas: ${OBJ}
	${CC} -o $@ ${OBJ} ${LDFLAGS}

clean:
	rm -f exas ${OBJ}

install: all
	mkdir -p ${DESTDIR}${PREFIX}/bin
	cp exas ${DESTDIR}${PREFIX}/bin
	chmod 4755 ${DESTDIR}${PREFIX}/bin/exas

	# Man Page
	mkdir -p  ${DESTDIR}${MANPREFIX}/man1
	cp exas.1 ${DESTDIR}${MANPREFIX}/man1/exas.1
	chmod 644 ${DESTDIR}${MANPREFIX}/man1/exas.1

uninstall:
	rm -f ${DESTDIR}${PREFIX}/bin/exas\
			${DESTDIR}${MANPREFIX}/man1/exas.1

.PHONY: all options clean install uninstall
