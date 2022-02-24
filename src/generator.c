#include "generator.h"

#include "lexer.h"

static int aout(const char* fmt, ...) {
    va_list args;

    va_start(args, fmt);
    int result = vfprintf(stdout, fmt, args);
    va_end(args);

    return result;
}

void cg_const(const Token* token) {
    aout("const long %.*s = ", token->len, token->start);
}

void cg_var(const Token* token) {
    aout("long %.*s", token->len, token->start);
}

void cg_call(const Token* token) {
    aout("%.*s()", token->len, token->start);
}

void cg_symbol(const Token* token) {
    switch (token->type) {
    case TOK_IDENT:
    case TOK_NUMBER:    aout("%.*s", token->len, token->start); break;
    case TOK_BEGIN:     aout("{\n"); break;
    case TOK_END:       aout("}\n"); break;
    case TOK_IF:        aout("if("); break;
    case TOK_THEN:
    case TOK_DO:        aout(") "); break;
    case TOK_ODD:       aout("("); break;
    case TOK_WHILE:     aout("while("); break;
    case TOK_EQUAL:     aout("=="); break;
    case TOK_COMMA:     aout(","); break;
    case TOK_ASSIGN:    aout("="); break;
    case TOK_HASH:      aout("!="); break;
    case TOK_LESSTHAN:  aout("<"); break;
    case TOK_GREATERTHAN: aout(">"); break;
    case TOK_PLUS:      aout("+"); break;
    case TOK_MINUS:     aout("-"); break;
    case TOK_MULTIPLY:  aout("*"); break;
    case TOK_DIVIDE:    aout("/"); break;
    case TOK_LPAREN:    aout("("); break;
    case TOK_RPAREN:    aout(")");
    }
}

void cg_procedure(int proc, const Token* token) {
    if (proc == 0) {
        aout("int main(int argc, char *argv[]) {\n");
    } else {
        aout("void %.*s() {\n", token->len, token->start);
    }
}

void cg_epilogue(int proc) {
    if (proc == 0) aout("return 0;\n");
    aout("}\n\n");
}

void cg_odd() {
    aout(")&1");
}

void cg_semicolon() {
    aout(";\n");
}

void cg_end(void) {
    aout("/* PL/0 compiler %s */\n", PL0C_VERSION);
}