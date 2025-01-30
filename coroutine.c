#include <assert.h>
#include <stdlib.h>

// TODO: make the STACK_CAPACITY customizable by the user
//#define STACK_CAPACITY (4*1024)
#define STACK_CAPACITY (4*1024*1024)

// Initial capacity of a dynamic array
#ifndef DA_INIT_CAP
#define DA_INIT_CAP 256
#endif

// Append an item to a dynamic array
#define da_append(da, item)                                                          \
    do {                                                                             \
        if ((da)->count >= (da)->capacity) {                                         \
            (da)->capacity = (da)->capacity == 0 ? DA_INIT_CAP : (da)->capacity*2;   \
            (da)->items = realloc((da)->items, (da)->capacity*sizeof(*(da)->items)); \
            assert((da)->items != NULL && "Buy more RAM lol");                       \
        }                                                                            \
                                                                                     \
        (da)->items[(da)->count++] = (item);                                         \
    } while (0)

typedef struct {
    void *rsp;
    void *stack_base;
} Context;

typedef struct {
    Context *items;
    size_t count;
    size_t capacity;
    size_t current;
} Contexts;

Contexts contexts = {0};

// TODO: ARM support
//   Requires modifications in all the @arch places

void __attribute__((naked)) coroutine_yield(void)
{
    // @arch
    asm(
    "    pushq %rdi\n"
    "    pushq %rbp\n"
    "    pushq %rbx\n"
    "    pushq %r12\n"
    "    pushq %r13\n"
    "    pushq %r14\n"
    "    pushq %r15\n"
    "    movq %rsp, %rdi\n"
    "    jmp coroutine_switch_context\n");
}

void __attribute__((naked)) coroutine_restore_context(void *rsp)
{
    // @arch
    (void)rsp;
    asm(
    "    movq %rdi, %rsp\n"
    "    popq %r15\n"
    "    popq %r14\n"
    "    popq %r13\n"
    "    popq %r12\n"
    "    popq %rbx\n"
    "    popq %rbp\n"
    "    popq %rdi\n"
    "    ret\n");
}

void coroutine_switch_context(void *rsp)
{
    contexts.items[contexts.current].rsp = rsp;
    contexts.current = (contexts.current + 1)%contexts.count;
    coroutine_restore_context(contexts.items[contexts.current].rsp);
}

void coroutine_init(void)
{
    da_append(&contexts, (Context){0});
}

void coroutine_finish(void)
{
    // TODO: Reuse the stack for new coroutines.
    // TODO: By removing elements from the contexts array we invalidate ids.

    if (contexts.current == 0) {
        contexts.count = 0;
        return;
    }

    Context t = contexts.items[contexts.current];
    contexts.items[contexts.current] = contexts.items[contexts.count-1];
    contexts.items[contexts.count-1] = t;
    contexts.count -= 1;
    contexts.current %= contexts.count;
    coroutine_restore_context(contexts.items[contexts.current].rsp);
}

void coroutine_go(void (*f)(void*), void *arg)
{
    // TODO: Mark the page at the end of the stack buffer as non-readable, non-writable, non-executable to make stack overflows of coroutines more obvious in the debugger
    //   This may require employing mmap(2) and mprotect(2) on Linux.
    void *stack_base = malloc(STACK_CAPACITY); // TODO: align the stack to 16 bytes or whatever
    void **rsp = (void**)((char*)stack_base + STACK_CAPACITY);
    // @arch
    *(--rsp) = coroutine_finish;
    *(--rsp) = f;
    *(--rsp) = arg; // push rdi
    *(--rsp) = 0;   // push rbx
    *(--rsp) = 0;   // push rbp
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
    return contexts.count;
}
