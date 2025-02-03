// Almost the same as counter.c but in C3
module counter_c3;
import coroutine;
import std::io;

fn void counter(void* arg) {
    usz n = (usz)arg;
    for (usz i = 0; i < n; ++i) {
        io::printfn("[%d] %d", coroutine::id(), i);
        coroutine::yield();
    }
}

fn void main() {
    coroutine::init();
    coroutine::go(fn void(void *arg) {
        io::printfn("[%d] Hello from C3 Lambda", coroutine::id());
    });
    coroutine::go(&counter, (void*)5);
    coroutine::go(&counter, (void*)10);
    while (coroutine::alive() > 1) coroutine::yield();
}
