all: src/v3.c
	gcc -pthread -o cracker src/v3.c -std=c99

debug: src/v3.C
	gcc -g -Wall -pthread -o Debug src/v3.c -std=c99

clean:
	rm cracker

tests: src/v3Func_test.c
	gcc -Wall -c src/v3Func.c
	gcc -Wall -pthread -L/usr/local/lib -o src/v3Func_test src/v3Func_test.c src/v3Func.o -lcunit -std=c99