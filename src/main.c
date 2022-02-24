#include "lexer.h"
#include "generator.h"
#include "symbols.h"

/*
 * pl0c -- PL/0 compiler.
 *
 * program  = block "." .
 * block    = [ "const" ident "=" number { "," ident "=" number } ";" ]
 *        [ "var" ident { "," ident } ";" ]
 *        { "procedure" ident ";" block ";" } statement .
 * statement    = [ ident ":=" expression
 *        | "call" ident
 *        | "begin" statement { ";" statement } "end"
 *        | "if" condition "then" statement
 *        | "while" condition "do" statement ] .
 * condition    = "odd" expression
 *        | expression ( "=" | "#" | "<" | ">" ) expression .
 * expression   = [ "+" | "-" ] term { ( "+" | "-" ) term } .
 * term     = factor { ( "*" | "/" ) factor } .
 * factor   = ident
 *        | number
 *        | "(" expression ")" .
 */

static Token token = { .line = 1 };
static int depth = 0;
static int proc = 0;
const char* stream;

symbol* symbols;

/*
 * Misc. functions.
 */


/*
 * Parser.
 */
static void expression();

static void next() {
    stream = lex(stream, &token);
}

static void expect(int type) {
    if (type != token.type) {
        errorAt(token.line, "syntax error (expected '%c' but got '%c')", type, token.type);
    }
    next();
}

static int consume(int type) {
    if (type == token.type) {
        next();
        return 1;
    }
    return 0;
}

static void factor() {
    switch (token.type) {
    case TOK_IDENT:
        symbolsCheck(symbols, &token, CHECK_RHS);
    case TOK_NUMBER:
        cg_symbol(&token);
        next();
        break;
    case TOK_LPAREN:
        cg_symbol(&token);
        expect(TOK_LPAREN);
        expression();
        if (token.type == TOK_RPAREN)
            cg_symbol(&token);
        expect(TOK_RPAREN);
    }
}

static void term(void) {
    factor();

    while (token.type == TOK_MULTIPLY || token.type == TOK_DIVIDE) {
        cg_symbol(&token);
        next();
        factor();
    }
}

static void expression(void) {
    if (token.type == TOK_PLUS || token.type == TOK_MINUS) {
        cg_symbol(&token);
        next();
    }

    term();

    while (token.type == TOK_PLUS || token.type == TOK_MINUS) {
        cg_symbol(&token);
        next();
        term();
    }
}

static void condition() {
    if (token.type == TOK_ODD) {
        cg_symbol(&token);
        expect(TOK_ODD);
        expression();
        cg_odd();
    } else {
        expression();

        switch (token.type) {
        case TOK_EQUAL:
        case TOK_HASH:
        case TOK_LESSTHAN:
        case TOK_GREATERTHAN:
            cg_symbol(&token);
            next();
            break;
        default:
            errorAt(token.line, "invalid conditional");
        }

        expression();
    }
}

static void statement() {
    switch (token.type) {
    case TOK_IDENT:
        symbolsCheck(symbols, &token, CHECK_LHS);
        cg_symbol(&token);
        expect(TOK_IDENT);
        if (token.type == TOK_ASSIGN)
            cg_symbol(&token);
        expect(TOK_ASSIGN);
        expression();
        cg_semicolon();
        break;
    case TOK_CALL:
        expect(TOK_CALL);
        if (token.type == TOK_IDENT) {
            symbolsCheck(symbols, &token, CHECK_CALL);
            cg_call(&token);
        }
        expect(TOK_IDENT);
        cg_semicolon();
        break;
    case TOK_BEGIN:
        cg_symbol(&token);
        expect(TOK_BEGIN);
        statement();
        while (token.type == TOK_SEMICOLON) {
            expect(TOK_SEMICOLON);
            statement();
        }
        if (token.type == TOK_END)
            cg_symbol(&token);
        expect(TOK_END);
        break;
    case TOK_IF:
        cg_symbol(&token);
        expect(TOK_IF);
        condition();
        if (token.type == TOK_THEN)
            cg_symbol(&token);
        expect(TOK_THEN);
        statement();
        break;
    case TOK_WHILE:
        cg_symbol(&token);
        expect(TOK_WHILE);
        condition();
        if (token.type == TOK_DO)
            cg_symbol(&token);
        expect(TOK_DO);
        statement();
        break;
    }
}

static void parseConst() {
    if (token.type == TOK_IDENT) {
        symbolsAdd(symbols, &token, TOK_CONST, depth);
        cg_const(&token);
    }

    expect(TOK_IDENT);
    expect(TOK_EQUAL);

    if (token.type == TOK_NUMBER) {
        cg_symbol(&token);
    }
    expect(TOK_NUMBER);
    cg_semicolon();
}

static void parseVar() {
    if (token.type == TOK_IDENT) {
        symbolsAdd(symbols, &token, TOK_VAR, depth);
        cg_var(&token);
    }
    expect(TOK_IDENT);
    cg_semicolon();
}

static void block() {
    if (depth++ > 1) errorAt(token.line, "nesting depth exceeded");

    if (consume(TOK_CONST)) {
        parseConst();

        while (consume(TOK_COMMA)) parseConst();
        expect(TOK_SEMICOLON);
    }

    if (consume(TOK_VAR)) {
        parseVar();

        while (consume(TOK_COMMA)) parseVar();
        expect(TOK_SEMICOLON);
    }

    while (token.type == TOK_PROCEDURE) {
        proc = 1;

        expect(TOK_PROCEDURE);
        if (token.type == TOK_IDENT) {
            symbolsAdd(symbols, &token, TOK_PROCEDURE, depth);
            cg_procedure(proc, &token);
        }
        expect(TOK_IDENT);
        expect(TOK_SEMICOLON);

        block();

        expect(TOK_SEMICOLON);

        proc = 0;
        symbolsDestroy(symbols);
    }

    if (proc == 0) cg_procedure(proc, &token);

    statement();

    cg_epilogue(proc);

    if (--depth < 0) errorAt(token.line, "nesting depth fell below 0");
}

static void parse() {
    next();
    block();
    expect(TOK_DOT);

    if (token.type != 0) errorAt(token.line, "extra token at end of file");

    cg_end();
}

static void listToken() {
    do {
        next();
        printToken(token);
        printf("\n");
    } while (token.type > 0);
}


/*
 * Main.
 */

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fputs("usage: pl0c file.pl0\n", stderr);
        exit(1);
    }

    char* buffer = readFile(argv[1]);
    stream = buffer;

    symbols = symbolsInit();
    parse();

    free(buffer);

    return 0;
}