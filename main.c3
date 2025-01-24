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
