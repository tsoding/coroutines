main: main.o coroutine.o
	ld -o main main.o coroutine.o /usr/lib/crt1.o -lc -dynamic-linker /lib64/ld-linux-x86-64.so.2

main.o: main.c
	gcc -c main.c

coroutine.o: coroutine.asm
	fasm coroutine.asm
