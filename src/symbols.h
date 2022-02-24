#ifndef PL0_SYMBOLS_H
#define PL0_SYMBOLS_H

#include "lexer.h"

#define CHECK_LHS   0
#define CHECK_RHS   1
#define CHECK_CALL  2

typedef struct symbol symbol;

struct symbol {
    char* name;
    int type;
    int depth;
    symbol* next;
};

symbol* symbolsInit();
void symbolsAdd(symbol* head, Token* token, int type, int depth);
void symbolsDestroy(symbol* head);

void symbolsCheck(symbol* head, const Token* token, int check);

#endif
