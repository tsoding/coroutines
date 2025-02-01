#import "Basic";

coroutine :: #library "../build/coroutine";
coroutine_sleep_read :: (fd: int) #foreign coroutine;
coroutine_sleep_write :: (fd: int) #foreign coroutine;
coroutine_wake_up :: (id: int) #foreign coroutine;
coroutine_init :: () #foreign coroutine;
coroutine_finish :: () #foreign coroutine;
coroutine_yield :: () #foreign coroutine;
coroutine_go :: (f: (*void) #c_call, arg: *void = null) #foreign coroutine;
coroutine_id :: () -> int #foreign coroutine;
coroutine_alive :: () -> int #foreign coroutine;

counter :: (arg: *void) #c_call {
    push_context {
        n := cast(int) arg;
        for 0..n-1 {
            print("[%] %\n", coroutine_id(), it);
            coroutine_yield();
        }
    }
}

main :: () {
    coroutine_init();
    defer coroutine_finish();
    coroutine_go((arg: *void) #c_call {
        push_context {
            print("[%] Hello from Jai Lambda\n", coroutine_id());
        }
    });
    coroutine_go(counter, cast(*void) 5);
    coroutine_go(counter, cast(*void) 10);
    while coroutine_alive() > 1 {
        coroutine_yield();
    }
}

#run {
    #import "Compiler";
    set_build_options_dc(.{
        output_path = "../build/",
        output_executable_name = "counter_jai",
    });

    coroutine_init();
    defer coroutine_finish();
    coroutine_go((arg: *void) #c_call {
        push_context {
            for 0..5 {
                print("[%] You can actually use\n", coroutine_id());
                coroutine_yield();
            }
        }
    });
    coroutine_go((arg: *void) #c_call {
        push_context {
            for 0..5 {
                print("[%] coroutines at compile-time\n", coroutine_id());
                coroutine_yield();
            }
        }
    });
    while coroutine_alive() > 1 {
        coroutine_yield();
    }
}
