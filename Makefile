cflags = -Wall -Wextra -ggdb


main: main.c coroutine.o
	gcc $(cflags) -o main main.c coroutine.o

coroutine.o: coroutine.c coroutine.h
	gcc $(cflags) -c coroutine.c

main_c3: main.c3 coroutine.o coroutine.c3
	c3c compile main.c3 coroutine.c3 coroutine.o
