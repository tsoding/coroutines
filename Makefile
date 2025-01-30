.PHONY: examples
examples: main main_c3 echo

echo: examples/echo.c3 coroutine.o coroutine.c3
	cp coroutine.o echo.coroutine.o # c3c deletes the object files for some reason, so we make a copy to preserve the original
	c3c compile examples/echo.c3 echo.coroutine.o coroutine.c3

main: examples/main.c coroutine.o
	gcc -I. -Wall -Wextra -ggdb -o main examples/main.c coroutine.o

main_c3: examples/main.c3 coroutine.o coroutine.c3
	cp coroutine.o main.coroutine.o # c3c deletes the object files for some reason, so we make a copy to preserve the original
	c3c compile examples/main.c3 coroutine.c3 main.coroutine.o

coroutine.o: coroutine.c
	gcc -Wall -Wextra -ggdb -c coroutine.c
