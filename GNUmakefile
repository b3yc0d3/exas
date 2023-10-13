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
	mkdir -p ${DESTDIR}${PREFOX}/bin
	cp exas ${DESTDIR}${PREFOX}/bin
	chmod 4775 ${DESTDIR}${PREFOX}/bin/exas

	# Man Page
	mkdir -p ${DESTDIR}${PREFOX}/man1
	cp exas.1 ${DESTDIR}${PREFOX}/man1/exas.1
	chmod 644 ${DESTDIR}${PREFOX}/man1/exas.1

uninstall:
	rm -f ${DESTDIR}${PREFIX}/bin/exas\
			${DESTDIR}${PREFIX}/man1/exas.1

.PHONY: all options clean install uninstall
