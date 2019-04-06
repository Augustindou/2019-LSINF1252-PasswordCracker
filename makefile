all: src/v1.c
	gcc -o cracker src/v1.c

clean:
	rm cracker
