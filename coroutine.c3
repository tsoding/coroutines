// coroutine bindings for C3
module coroutine;

def CoroutineFn = fn void(void*);

extern fn void init() @extern("coroutine_init");
extern fn void finish() @extern("coroutine_finish");
extern fn void yield() @extern("coroutine_yield");
extern fn void go(CoroutineFn f, void* arg) @extern("coroutine_go");
extern fn usz id() @extern("coroutine_id");
extern fn usz alive() @extern("coroutine_alive");
