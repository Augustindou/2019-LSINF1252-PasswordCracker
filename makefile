all: src/v2.c
	gcc -pthread -o cracker src/v2.c -std=c99

clean:
	rm cracker