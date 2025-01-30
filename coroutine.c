#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

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

#define da_swap(type, da, i, j)              \
    do {                                     \
        type t = (da)->items[(i)];           \
        (da)->items[(i)] = (da)->items[(j)]; \
        (da)->items[(j)] = t;                \
    } while(0)

typedef struct {
    void *rsp;
    void *stack_base;
    bool dead;
} Context;

typedef struct {
    Context *items;
    size_t count;
    size_t capacity;
} Contexts;

typedef struct {
    size_t *items;
    size_t count;
    size_t capacity;
} Indices;

static size_t current     = 0;
static Indices active     = {0};
static Indices dead       = {0};
static Contexts contexts_ = {0};

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
    contexts_.items[active.items[current]].rsp = rsp;
    current = (current + 1)%active.count;
    coroutine_restore_context(contexts_.items[active.items[current]].rsp);
}

void coroutine_init(void)
{
    da_append(&contexts_, (Context){0});
    da_append(&active, 0);
}

void coroutine_finish(void)
{
    if (current == 0) {
        for (size_t i = 1; i < contexts_.count; ++i) {
            free(contexts_.items[i].stack_base);
        }
        free(contexts_.items);
        free(active.items);
        free(dead.items);
        memset(&contexts_, 0, sizeof(contexts_));
        memset(&active, 0, sizeof(active));
        memset(&dead, 0, sizeof(dead));
        return;
    }

    contexts_.items[active.items[current]].dead = true;
    da_append(&dead, current);
    da_swap(size_t, &active, current, active.count-1);
    active.count -= 1;
    current %= active.count;
    coroutine_restore_context(contexts_.items[active.items[current]].rsp);
}

void coroutine_go(void (*f)(void*), void *arg)
{
    size_t id;
    if (dead.count > 0) {
        id = dead.items[--dead.count];
        assert(contexts_.items[id].dead);
        contexts_.items[id].dead = false;
    } else {
        // TODO: Mark the page at the end of the stack buffer as non-readable, non-writable, non-executable to make stack overflows of coroutines more obvious in the debugger
        //   This may require employing mmap(2) and mprotect(2) on Linux.
        da_append(&contexts_, ((Context){0}));
        id = contexts_.count-1;
        contexts_.items[id].stack_base = malloc(STACK_CAPACITY); // TODO: align the stack to 16 bytes or whatever
    }

    void **rsp = (void**)((char*)contexts_.items[id].stack_base + STACK_CAPACITY);
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
    contexts_.items[id].rsp = rsp;

    da_append(&active, id);
}

size_t coroutine_id(void)
{
    return active.items[current];
}

size_t coroutine_alive(void)
{
    return active.count;
}
