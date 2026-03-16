tests_run: 
	./tests/out

test: 
	gcc -Wall tests/main.c tpool.c -I. -o tests/out 
	
tpool: 
	gcc -Wall -c tpool.c
