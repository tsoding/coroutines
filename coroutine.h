#ifndef COROUTINE_H_
#define COROUTINE_H_

// void coroutine_init(void);
void coroutine_yield(void);
void coroutine_add(void (*f)(void*), void *arg);
void coroutine_finish(void);

size_t coroutine_id(void);
size_t coroutine_alive(void);

#endif // COROUTINE_H_
