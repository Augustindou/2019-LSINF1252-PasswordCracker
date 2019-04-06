all: src/v1.c
	gcc -o cracker src/v1.c -std=c99

clean:
	rm cracker
