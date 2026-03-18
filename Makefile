tests_run: tests/out
	./tests/out

tests/out: tests/main.c tests/tpool.o
	gcc -Wall tests/main.c tests/tpool.o -I. -pthread -o tests/out

tests/tpool.o: tpool.c tpool.h
	gcc -Wall -c tpool.c -o tests/tpool.o
