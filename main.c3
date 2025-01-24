module main_c3;
import coroutine;
import std::io;

fn void counter(void* arg)
{
    usz n = (usz)arg;
    for (usz i = 0; i < n; ++i) {
        io::printfn("[%d] %d", coroutine::id(), i);
        coroutine::yield();
    }
}

fn void main() {
    coroutine::init();
    defer coroutine::finish();
    coroutine::go(&counter, (void*)20);
    coroutine::go(&counter, (void*)5);
    while (coroutine::alive() > 1) coroutine::yield();
}

module coroutine;

def CoroutineFn = fn void(void*);

extern fn void init() @extern("coroutine_init");
extern fn void finish() @extern("coroutine_finish");
extern fn void yield() @extern("coroutine_yield");
extern fn void go(CoroutineFn f, void* arg) @extern("coroutine_go");
extern fn usz id() @extern("coroutine_id");
extern fn usz alive() @extern("coroutine_alive");
