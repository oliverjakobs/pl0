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
        cgSymbol(&token);
        next();
        break;
    case TOK_LPAREN:
        cgSymbol(&token);
        expect(TOK_LPAREN);
        expression();
        if (token.type == TOK_RPAREN)
            cgSymbol(&token);
        expect(TOK_RPAREN);
    }
}

static void term(void) {
    factor();

    while (token.type == TOK_MULTIPLY || token.type == TOK_DIVIDE) {
        cgSymbol(&token);
        next();
        factor();
    }
}

static void expression(void) {
    if (token.type == TOK_PLUS || token.type == TOK_MINUS) {
        cgSymbol(&token);
        next();
    }

    term();

    while (token.type == TOK_PLUS || token.type == TOK_MINUS) {
        cgSymbol(&token);
        next();
        term();
    }
}

static void condition() {
    if (token.type == TOK_ODD) {
        cgSymbol(&token);
        expect(TOK_ODD);
        expression();
        cgOdd();
    } else {
        expression();

        switch (token.type) {
        case TOK_EQUAL:
        case TOK_HASH:
        case TOK_LESSTHAN:
        case TOK_GREATERTHAN:
            cgSymbol(&token);
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
        cgSymbol(&token);
        expect(TOK_IDENT);
        if (token.type == TOK_ASSIGN)
            cgSymbol(&token);
        expect(TOK_ASSIGN);
        expression();
        cgSemicolon();
        break;
    case TOK_CALL:
        expect(TOK_CALL);
        if (token.type == TOK_IDENT) {
            symbolsCheck(symbols, &token, CHECK_CALL);
            cgCall(&token);
        }
        expect(TOK_IDENT);
        cgSemicolon();
        break;
    case TOK_BEGIN:
        cgSymbol(&token);
        expect(TOK_BEGIN);
        statement();
        while (token.type == TOK_SEMICOLON) {
            expect(TOK_SEMICOLON);
            statement();
        }
        if (token.type == TOK_END)
            cgSymbol(&token);
        expect(TOK_END);
        break;
    case TOK_IF:
        cgSymbol(&token);
        expect(TOK_IF);
        condition();
        if (token.type == TOK_THEN)
            cgSymbol(&token);
        expect(TOK_THEN);
        statement();
        break;
    case TOK_WHILE:
        cgSymbol(&token);
        expect(TOK_WHILE);
        condition();
        if (token.type == TOK_DO)
            cgSymbol(&token);
        expect(TOK_DO);
        statement();
        break;
    case TOK_WRITE_INT:
        expect(TOK_WRITE_INT);
        if (token.type == TOK_IDENT || token.type == TOK_NUMBER) {
            if (token.type == TOK_IDENT) symbolsCheck(symbols, &token, CHECK_RHS);
            cgWriteInt(&token);
            next();
        } else {
            errorAt(token.line, "writeInt takes an identifier or a number");
        }
        cgSemicolon();
        break;
    case TOK_WRITE_CHAR:
        expect(TOK_WRITE_CHAR);
        if (token.type == TOK_IDENT || token.type == TOK_NUMBER) {
            if (token.type == TOK_IDENT) symbolsCheck(symbols, &token, CHECK_RHS);
            cgWriteChar(&token);
            next();
        } else {
            errorAt(token.line, "writeChar takes an identifier or a number");
        }
        cgSemicolon();
        break;
    }
}

static void parseConst() {
    if (token.type == TOK_IDENT) {
        symbolsAdd(symbols, &token, TOK_CONST, depth);
        cgConst(&token);
    }

    expect(TOK_IDENT);
    expect(TOK_EQUAL);

    if (token.type == TOK_NUMBER) {
        cgSymbol(&token);
    }
    expect(TOK_NUMBER);
    cgSemicolon();
}

static void parseVar() {
    if (token.type == TOK_IDENT) {
        symbolsAdd(symbols, &token, TOK_VAR, depth);
        cgVar(&token);
    }
    expect(TOK_IDENT);
    cgSemicolon();
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
            cgProcedure(proc, &token);
        }
        expect(TOK_IDENT);
        expect(TOK_SEMICOLON);

        block();

        expect(TOK_SEMICOLON);

        proc = 0;
        symbolsDestroy(symbols);
    }

    if (proc == 0) cgProcedure(proc, &token);

    statement();

    cgEpilogue(proc);

    if (--depth < 0) errorAt(token.line, "nesting depth fell below 0");
}

static void parse() {
    next();
    block();
    expect(TOK_DOT);

    if (token.type != 0) errorAt(token.line, "extra token at end of file");

    cgEnd();
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
    cgInit();
    parse();

    free(buffer);

    return 0;
}