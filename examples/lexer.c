// Lexer is a classical example of usecase for coroutines.
// This is a *very* simple and basic lexer that
// can lex single digit integers, + and -.
// The example would be better if we could return values
// when we yield (kind of like a generator). But it is what it is.

#include <stdio.h>
#include <assert.h>
#include <stdbool.h>

#include <coroutine.h>

typedef enum {
    TK_INT,
    TK_OP,
    TK_EOF
} TokenKind;

typedef union {
    char tk_op;
    int  tk_int;
} TokenValue;

TokenKind  token_kind  = TK_EOF;
TokenValue token_value = {0};

void lex(void* input_void) {
    if (input_void == NULL) return;

    const char* input = input_void;

    while(true) {
        switch(*input) {
            // Numba
            case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9': {
                token_kind = TK_INT;
                token_value.tk_int = *input - '0';
            } break;

            // Operators
            case '+': case '-': {
                token_kind = TK_OP;
                token_value.tk_op = *input;
            } break;

            default: {
                token_kind = TK_EOF;
                return;
            }
        }
        input++;

        // For every token we consume, we yield control back to the caller (a parser, I guess).
        coroutine_yield();
    }
}

int main(int argc, char* argv[]){
    if (argc != 2) {
        printf("Usage: %s <input-text>\n", argv[0]);
        return 1;
    }

    coroutine_init();
    {
        coroutine_go(lex, argv[1]);

        // Consume those tokens
        bool quit = false;
        while(!quit && coroutine_alive() > 1){
            // Yield control to the lexer.
            // It will lex and yield control back to here.
            coroutine_yield();
            switch(token_kind){
                case TK_INT: { printf("TK_INT: %d\n", token_value.tk_int); } break;
                case TK_OP:  { printf("TK_OP:  %c\n", token_value.tk_op); } break;
                default:     { printf("Done!\n"); quit = true; } break;
            }
        }
    }
    coroutine_finish();

    return 0;
}
