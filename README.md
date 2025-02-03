# Coroutines

> [!WARNING]
> The library is not in production ready state yet

Custom coroutines implementation in C.

## What is a Coroutine?

<!-- This is almost an exact copy of "What is a Coroutine?" section from ./examples/counter.c, but it's slightly modified to make more since on the front README page of a GitHub Repo -->

Coroutine is a lightweight user space thread with its own stack that can suspend its execution and switch to another coroutine on demand. Coroutines do not run in parallel but rather cooperatively switch between each other whenever they feel like it.

Coroutines are useful in cases when all your program does majority of the time is waiting on IO. So with coroutines you have an opportunity to switch the context and go do something else. It is not useful to split up heavy CPU computations because they all going to be executed on a single thread. Use proper threads for that (pthreads on POSIX).

Good use cases for coroutines are usually Network Applications and UI. Anything with a slow Async IO.

<!-- End of the copy of the section -->

See [coroutine.h](./coroutine.h) for more info. See [./examples/counter.c](./examples/counter.c) for a simple usage example in C.

To build the example:

```console
$ make
$ ./build/counter
```

For Windows you need to use clang

```console
$ make cc=clang
$ ./build/counter.exe
```

There are actually much more examples in the [./examples/](./examples/) in a variety of languages. To build all of them do:

```console
$ make examples
```

Make sure you have all the corresponding compilers for the languages.

## Supported platforms

- Linux   x86_64
- Windows x86_64 (tested for Msys clang)

*More are planned in the future*

## Special Thanks

This library is heavily inspired by [libmill](https://libmill.org/) and [libdill](https://libdill.org/).
