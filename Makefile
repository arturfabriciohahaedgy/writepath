CC = cc
CFLAGS = -Wall -Wextra -g -pedantic -std=c99 -pipe -O2

writepath: writepath.c
	${CC} ${CFLAGS} writepath.c -o wp
