all: src/v3.c
	gcc -Wall -pthread -o cracker src/v3.c -std=c99

clean:
	rm cracker