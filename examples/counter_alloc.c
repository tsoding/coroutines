#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

void *mya_malloc    (size_t size);
void  mya_free      (void *ptr);
void *mya_realloc   (void *ptr, size_t size);

#define COROUTINE_MALLOC    mya_malloc
#define COROUTINE_REALLOC   mya_realloc
#define COROUTINE_FREE      mya_free

#include "coroutine.h"

void counter(void *arg)
{
    long int n = (long int)arg;
    for (int i = 0; i < n; ++i) {
        printf("[%zu] %d\n", coroutine_id(), i);
        coroutine_yield();
    }
}

int main()
{
    coroutine_init();
        coroutine_go(&counter, (void*)5);
        coroutine_go(&counter, (void*)10);
        while (coroutine_alive() > 1) coroutine_yield();
    coroutine_finish();
    return 0;
}
