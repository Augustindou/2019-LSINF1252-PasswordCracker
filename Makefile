flags = -Wall -Werror
libs = -lpthread -lrt -lm -std=c99


#cracker
cracker: src/main.c
	cc $(flags) -o cracker src/main.c $(libs)

#all
all: cracker tests

#tests
tests: functions.o functions_tests.o
	rm src/functions.o
	rm test

functions.o:src/functions.c
	cc $(flags) -c src/functions.c $(libs) -o src/functions.o

functions_tests.o:tests/functions_tests.c
	cc $(flags) -o test tests/functions_tests.c -lcunit $(libs)
	./test

#debug
debug: src/main.c
	cc $(flags) -o debug src/main.c $(libs)
#clean
clean:
	rm -f cracker
	rm -f debug










