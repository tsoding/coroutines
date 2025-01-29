.PHONY: all
all: main main_c3

main: main.c coroutine.o
	gcc -Wall -Wextra -ggdb -o main main.c coroutine.o

coroutine.o: coroutine.c
	gcc -Wall -Wextra -ggdb -c coroutine.c

main_c3: main.c3 coroutine.o coroutine.c3
	c3c compile main.c3 coroutine.c3 coroutine.o
