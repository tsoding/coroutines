build/counter: examples/counter.c coroutine.h build/coroutine.a
	gcc -I. -Wall -Wextra -ggdb -o build/counter examples/counter.c build/coroutine.a

.PHONY: examples
examples: build/counter build/counter_cpp build/counter_c3 build/counter_jai build/echo build/lexer

build/echo: examples/echo.c3 coroutine.c3 build/coroutine.a
	c3c compile -l build/coroutine.a -o build/echo examples/echo.c3 coroutine.c3

build/counter_cpp: examples/counter.cpp coroutine.h build/coroutine.a
	g++ -I. -Wall -Wextra -ggdb -o build/counter_cpp examples/counter.cpp build/coroutine.a

build/counter_c3: examples/counter.c3 coroutine.c3 build/coroutine.a
	c3c compile -l build/coroutine.a -o build/counter_c3 examples/counter.c3 coroutine.c3

build/counter_jai: examples/counter.jai build/coroutine.a build/coroutine.so
	jai-linux examples/counter.jai

build/lexer: examples/lexer.c coroutine.h build/coroutine.a
	gcc -I. -Wall -Wextra -ggdb -o build/lexer examples/lexer.c build/coroutine.a

build/coroutine.so: coroutine.c
	mkdir -p build
	gcc -Wall -Wextra -ggdb -shared -fPIC -o build/coroutine.so coroutine.c

build/coroutine.a: build/coroutine.o
	ar -rcs build/coroutine.a build/coroutine.o

build/coroutine.o: coroutine.c coroutine.h
	mkdir -p build
	gcc -Wall -Wextra -ggdb -c -o build/coroutine.o coroutine.c
