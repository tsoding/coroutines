format ELF64

STDIN_FILENO = 0
STDOUT_FILENO = 1
STDERR_FILENO = 2

SYS_write = 1
SYS_exit = 60

COROUTINES_CAPACITY = 10
STACK_CAPACITY = 4*1024

;; TODO: make coroutine.o relocatable

section '.text'
;; rdi - procedure to start in a new coroutine
public coroutine_go
coroutine_go:
    cmp QWORD [contexts_count], COROUTINES_CAPACITY
    jge overflow_fail

    mov rbx, [contexts_count]     ;; rbx contains the index of the
                                  ;; context we just allocated
    inc QWORD [contexts_count]

    mov rax, [stacks_end]         ;; rax contains the rsp of the new
                                  ;; routine
    sub QWORD [stacks_end], STACK_CAPACITY
    sub rax, 8
    mov QWORD [rax], coroutine_finish

    mov [contexts_rsp+rbx*8], rax
    mov QWORD [contexts_rbp+rbx*8], 0
    mov [contexts_rip+rbx*8], rdi

    ret

;; ...[ret]
;;         ^
public coroutine_init
coroutine_init:
    cmp QWORD [contexts_count], COROUTINES_CAPACITY
    jge overflow_fail

    mov rbx, [contexts_count]     ;; rbx contains the index of the
                                  ;; context we just allocated
    inc QWORD [contexts_count]

    pop rax                       ;; return address is in rax now
    mov [contexts_rsp+rbx*8], rsp
    mov [contexts_rbp+rbx*8], rbp
    mov [contexts_rip+rbx*8], rax

    jmp rax

;; ...[ret]
;;         ^
public coroutine_yield
coroutine_yield:
    mov rbx, [contexts_current]

    pop rax                       ;; return address is in rax now
    mov [contexts_rsp+rbx*8], rsp
    mov [contexts_rbp+rbx*8], rbp
    mov [contexts_rip+rbx*8], rax

    inc rbx
    xor rcx, rcx
    cmp rbx, [contexts_count]
    cmovge rbx, rcx
    mov [contexts_current], rbx

    mov rsp, [contexts_rsp+rbx*8]
    mov rbp, [contexts_rbp+rbx*8]
    jmp QWORD [contexts_rip+rbx*8]

coroutine_finish:
    mov rax, SYS_write
    mov rdi, STDERR_FILENO
    mov rsi, coroutine_finish_not_implemented
    mov rdx, coroutine_finish_not_implemented_len
    syscall

    mov rax, SYS_exit
    mov rdi, 69
    syscall

overflow_fail:
    mov rax, SYS_write
    mov rdi, STDERR_FILENO
    mov rsi, too_many_coroutines_msg
    mov rdx, too_many_coroutines_msg_len
    syscall

    mov rax, SYS_exit
    mov rdi, 69
    syscall

section '.data'
too_many_coroutines_msg: db "ERROR: Too many coroutines", 0, 10
too_many_coroutines_msg_len = $-too_many_coroutines_msg
coroutine_finish_not_implemented: db "TODO: coroutine_finish is not implemented", 0, 10
coroutine_finish_not_implemented_len = $-coroutine_finish_not_implemented
ok: db "OK", 0, 10
ok_len = $-ok

contexts_current: dq 0
stacks_end:       dq stacks+COROUTINES_CAPACITY*STACK_CAPACITY

section '.bss'
stacks:           rb COROUTINES_CAPACITY*STACK_CAPACITY
contexts_rsp:     rq COROUTINES_CAPACITY
contexts_rbp:     rq COROUTINES_CAPACITY
contexts_rip:     rq COROUTINES_CAPACITY
public contexts_count
contexts_count:   rq 1
