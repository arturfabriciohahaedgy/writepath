CC = cc
CFLAGS = -Wall -Wextra -g -pedantic

writepath: main.c
	${CC} ${CFLAGS} main.c -o writepath.out
