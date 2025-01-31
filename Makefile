build/counter: examples/counter.c coroutine.h build/coroutine.o
	gcc -I. -Wall -Wextra -ggdb -o build/counter examples/counter.c build/coroutine.o

.PHONY:
examples: build/counter build/counter_cpp build/counter_c3 build/echo

build/echo: examples/echo.c3 coroutine.c3 build/coroutine.o
	cp build/coroutine.o build/echo.coroutine.o # c3c deletes the object files for some reason, so we make a copy to preserve the original
	c3c compile -o build/echo examples/echo.c3 coroutine.c3 build/echo.coroutine.o

build/counter_cpp: examples/counter.cpp coroutine.h build/coroutine.o
	g++ -I. -Wall -Wextra -ggdb -o build/counter_cpp examples/counter.cpp build/coroutine.o

build/counter_c3: examples/counter.c3 coroutine.c3 build/coroutine.o
	cp build/coroutine.o build/counter.coroutine.o # c3c deletes the object files for some reason, so we make a copy to preserve the original
	c3c compile -o build/counter_c3 examples/counter.c3 coroutine.c3 build/counter.coroutine.o

build/coroutine.o: coroutine.c coroutine.h
	mkdir -p build
	gcc -Wall -Wextra -ggdb -c -o build/coroutine.o coroutine.c

