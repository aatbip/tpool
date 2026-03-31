CC = gcc
CC_FLAGS = -Wall -O2 -I. -pthread

tests/tpool.o: tpool.c tpool.h
	${CC} -Wall -c $< -o $@

tests/out: tests/$(EX) tests/tpool.o
	${CC} ${CC_FLAGS} $^ -o $@

run: clean tests/out
	@echo "\n-------Running ./tests/$(EX)-------\n"
	./tests/out

clean: 
	rm -rf ./tests/out && rm -rf ./tests/tpool.o
