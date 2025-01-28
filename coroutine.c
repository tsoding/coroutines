#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>

#ifndef DA_INIT_CAP
#define DA_INIT_CAP 256
#endif

#define da_append(da, item)                                                          \
    do {                                                                             \
        if ((da)->count >= (da)->capacity) {                                         \
            (da)->capacity = (da)->capacity == 0 ? DA_INIT_CAP : (da)->capacity*2;   \
            (da)->items = realloc((da)->items, (da)->capacity*sizeof(*(da)->items)); \
            assert((da)->items != NULL && "Buy more RAM lol");                       \
        }                                                                            \
        (da)->items[(da)->count++] = (item);                                         \
    } while (0)

typedef struct {
    void *rsp;              
    void *stack_base;       
    void *allocated_base;   
} Context;

typedef struct {
    Context *items;
    size_t count;
    size_t capacity;
    size_t current;
} Contexts;

Contexts contexts = {0};

typedef struct {
    Context **items;
    size_t count;
    size_t capacity;
} FreedStacks;

FreedStacks freed_stacks = {0};

void __attribute__((naked)) coroutine_yield(void)
{
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
    contexts.current = (contexts.current + 1) % contexts.count;
    coroutine_restore_context(contexts.items[contexts.current].rsp);
}

void coroutine_init(void)
{
    da_append(&contexts, (Context){0});
}

#define STACK_CAPACITY (4*1024)

void coroutine_finish(void)
{
    da_append(&freed_stacks, contexts.items[contexts.current].allocated_base);

    if (contexts.current == 0) {
        contexts.count = 0;

        for (size_t i = 0; i < freed_stacks.count; i++) {
            free(freed_stacks.items[i]);
        }

        freed_stacks.count = 0;
        return;
    }

    Context t = contexts.items[contexts.current];
    contexts.items[contexts.current] = contexts.items[contexts.count - 1];
    contexts.items[contexts.count - 1] = t;
    contexts.count -= 1;
    contexts.current %= contexts.count;
    coroutine_restore_context(contexts.items[contexts.current].rsp);
}

void coroutine_go(void (*f)(void*), void *arg)
{
    void *raw = malloc(STACK_CAPACITY + 16);
    uintptr_t addr = (uintptr_t)raw;
    addr = (addr + 15) & ~(uintptr_t)15;
    void *stack_base = (void*)addr;

    void **rsp = (void**)((char*)stack_base + STACK_CAPACITY);

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
        .allocated_base = raw
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
