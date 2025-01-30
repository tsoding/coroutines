.PHONY: examples
examples: counter counter_c3 echo

echo: examples/echo.c3 coroutine.o coroutine.c3
	cp coroutine.o echo.coroutine.o # c3c deletes the object files for some reason, so we make a copy to preserve the original
	c3c compile examples/echo.c3 echo.coroutine.o coroutine.c3

counter: examples/counter.c coroutine.o
	gcc -I. -Wall -Wextra -ggdb -o counter examples/counter.c coroutine.o

counter_c3: examples/counter.c3 coroutine.o coroutine.c3
	cp coroutine.o counter.coroutine.o # c3c deletes the object files for some reason, so we make a copy to preserve the original
	c3c compile examples/counter.c3 coroutine.c3 counter.coroutine.o

coroutine.o: coroutine.c
	gcc -Wall -Wextra -ggdb -c coroutine.c
