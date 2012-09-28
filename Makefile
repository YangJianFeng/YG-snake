#Makefile

CC=gcc
MAIN_NAME=snake
WIN_NAME=snake-gui
MSOURCES=module.c snake.c
WSOURCES=module.c winsnake.c
HEADS=public.h snake.h winsnake.h
LIBS=-lpthread -lncursesw
WALL=-Wall
INSPATH=/usr/local/ygsnake
LNNAME=/usr/bin/ygsnake
MOBJS=${MSOURCES:.c=.o}
WOBJS=${WSOURCES:.c=.o}

INSPATH=/usr/local/ygsnake

${MAIN_NAME}:${MOBJS} ${WIN_NAME}
	${CC} -o ${MAIN_NAME} ${LIBS} ${WALL} ${MOBJS}
${WIN_NAME}:${WOBJS}
	${CC} -o ${WIN_NAME} ${LIBS} ${WALL} ${WOBJS}

${MOBJS}:${HEADS}

%o:%c
	${CC} -c $<

install:
	mkdir ${INSPATH}
	mv ${MAIN_NAME} ${WIN_NAME} ${INSPATH}
	@ln -s ${INSPATH}/snake ${LNNAME}

clear:
	rm -rf ${INSPATH} ${LNNAME}

