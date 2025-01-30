#include <assert.h>
#include <stdlib.h>

// Initial capacity of a dynamic array
#ifndef DA_INIT_CAP
#define DA_INIT_CAP 256
#endif

// Append an item to a dynamic array
#define da_append(da, item)                                                          \
    do {                                                                             \
        if ((da)->count >= (da)->capacity) {                                         \
            (da)->capacity = (da)->capacity == 0 ? DA_INIT_CAP : (da)->capacity*2;   \
            (da)->items = reallocarray((da)->items, (da)->capacity, sizeof(item));   \
            assert((da)->items != NULL && "Buy more RAM lol");                       \
        }                                                                            \
                                                                                     \
        (da)->items[(da)->count++] = (item);                                         \
    } while (0)


typedef struct {
    void *rsp;
    const void *stack_base;
} Context;

typedef struct {
    Context *items;
    size_t count;
    size_t capacity;
    size_t current;
} Contexts;

Contexts contexts = {0};

void __attribute__((naked)) coroutine_yield(void)
{
    asm (
    "    cmpq $2, %0\n"
    "    jbe just_ret\n"
    "    pushq %%rdi\n"  // save the registers
    "    pushq %%rbp\n"
    "    pushq %%rbx\n"
    "    pushq %%r12\n"
    "    pushq %%r13\n"
    "    pushq %%r14\n"
    "    pushq %%r15\n"
    "    movq %%rsp, %%rdi\n"  // make %rdi point to our stack
    // so coroutine_switch_context takes that %rsp as param
    "    jmp coroutine_switch_context\n"
    "just_ret:\n"
    "    ret\n"
    :  // output
    : "m" (contexts.count)  // input
    );
}

void __attribute__((naked)) coroutine_restore_context(void *rsp)
{
    (void)rsp;  // just shut up compiler
    asm(
    "    movq %rdi, %rsp\n"  // restore the stack from param
    "    popq %r15\n"  // restore the registers
    "    popq %r14\n"
    "    popq %r13\n"
    "    popq %r12\n"
    "    popq %rbx\n"
    "    popq %rbp\n"
    "    popq %rdi\n"  // restore the argument to "counter"
    "    ret\n");  // return to the "counter" function
}

void coroutine_switch_context(void *rsp)
{
    contexts.items[contexts.current].rsp = rsp;  // we save what we got from %rdi
    contexts.current = (contexts.current + 1) % contexts.count;
    if (contexts.current == 0 && contexts.count > 1) {
        contexts.current = 1;
    }
    coroutine_restore_context(contexts.items[contexts.current].rsp);
    abort();  // unreachable code
}

void coroutine_init(void)
{
    da_append(&contexts, (Context){0});
}

// 4096 = pow(2, 12)
#define STACK_CAPACITY (4*1024)
#define STACK_ALIGNMENT (sizeof(void*) == 8 ? 16 : 8)

void coroutine_finish(void)
{
    if (contexts.current == 0) {
        contexts.count = 0;
        // when current is 0, our stack is not on the heap
        free(contexts.items);
        return;
    }

    Context t = contexts.items[contexts.current];
    free((void*)t.stack_base);
    contexts.count -= 1;
    contexts.current %= contexts.count;
    if (contexts.current == 0 && contexts.count > 1) {
        contexts.current = 1;
    }
    coroutine_restore_context(contexts.items[contexts.current].rsp);
}

void coroutine_add(void (* const f)(void*), void *arg)
{
    /* Tell the coroutine where to go: call f(arg) then coroutine_finish */
    if (contexts.count == 0)
        coroutine_init();
    assert(STACK_CAPACITY % STACK_ALIGNMENT == 0);
    // XXX: implement base 2 logarithm to test STACK_ALIGNMENT is a power of 2
    assert(STACK_CAPACITY % sizeof(void*) == 0);
    const void *stack_base = aligned_alloc(STACK_ALIGNMENT, STACK_CAPACITY);
    void **rsp = (void**)((char*)stack_base + STACK_CAPACITY);
    *(--rsp) = coroutine_finish;
    *(--rsp) = f;
    *(--rsp) = arg; // push rdi
    *(--rsp) = 0;   // push rbx
    *(--rsp) = (void*)stack_base;   // push rbp
    *(--rsp) = 0;   // push r12
    *(--rsp) = 0;   // push r13
    *(--rsp) = 0;   // push r14
    *(--rsp) = 0;   // push r15
    da_append(&contexts, ((Context){
        .rsp = rsp,
        .stack_base = stack_base,
    }));
}

size_t coroutine_id(void)
{
    return contexts.current;
}

size_t coroutine_alive(void)
{
    return contexts.count - 1;
}
