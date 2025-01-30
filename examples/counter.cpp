// Almost the same as counter.c but in C++
#include <iostream>
#include "coroutine.h"

void counter(void *arg)
{
    size_t n = reinterpret_cast<size_t>(arg);
    for (size_t i = 0; i < n; ++i) {
        std::cout << "[" << coroutine_id() << "] " << i << std::endl;
        coroutine_yield();
    }
}

int main()
{
    coroutine_init();
    coroutine_go([](void*) {
        std::cout << "Hello from C++ Lambda" << std::endl;
    }, nullptr);
    coroutine_go(counter, reinterpret_cast<void*>(5));
    coroutine_go(counter, reinterpret_cast<void*>(10));
    while (coroutine_alive() > 1) coroutine_yield();
    coroutine_finish();
    return 0;
}
