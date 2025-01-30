#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "coroutine.h"

void counter(void *arg)
{
    long int n = (long int)arg;
    for (int i = 0; i < n; ++i) {
        printf("[%ld/%ld] %d\n", coroutine_id(), coroutine_alive(), i);
        coroutine_yield();
    }
}

int main()
{
    // Add the coroutines you want
    coroutine_add(&counter, (void*)10);
    coroutine_add(&counter, (void*)3);
    // Yield to start the first coroutine
    coroutine_yield();
    printf("All of the coroutines reached their end\n");
    return 0;
}
