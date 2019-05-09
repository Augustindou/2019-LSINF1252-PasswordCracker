all: src/v3.c
	gcc -pthread -o cracker src/v3.c -std=c99

debug: src/v3.C
	gcc -g -Wall -pthread -o Debug src/v3.c -std=c99

clean:
	rm cracker
