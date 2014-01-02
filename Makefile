# Makefile for c
CC=clang
FLAGS=-g -Wall -Werror
OBJ=main.o
JVC=
main.o:
	${CC} ${FLAGS} -o main.o -c main.c
main: ${OBJ}
	${CC} ${FLAGS} -o main ${OBJ}
clean:
	rm ${OBJ} ${JVC} main
remake: clean main
