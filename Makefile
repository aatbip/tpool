CC = gcc
CC_FLAGS = -Wall -I. -pthread

tests/tpool.o: tpool.c tpool.h
	${CC} -Wall -c $< -o $@

tests/out: tests/main.c tests/tpool.o
	${CC} ${CC_FLAGS} $^ -o $@

run: tests/out
	./tests/out
