#ifndef PL0_LEXER_H
#define PL0_LEXER_H

#include "common.h"

#define TOK_IDENT       'I'
#define TOK_NUMBER      'N'
#define TOK_CONST       'C'
#define TOK_VAR         'V'
#define TOK_PROCEDURE   'P'
#define TOK_CALL        'c'
#define TOK_BEGIN       'B'
#define TOK_END         'E'
#define TOK_IF          'i'
#define TOK_THEN        'T'
#define TOK_WHILE       'W'
#define TOK_DO          'D'
#define TOK_ODD         'O'
#define TOK_WRITE_INT   'w'
#define TOK_WRITE_CHAR  'H'
#define TOK_DOT         '.'
#define TOK_EQUAL       '='
#define TOK_COMMA       ','
#define TOK_SEMICOLON   ';'
#define TOK_ASSIGN      ':'
#define TOK_HASH        '#'
#define TOK_LESSTHAN    '<'
#define TOK_GREATERTHAN '>'
#define TOK_PLUS        '+'
#define TOK_MINUS       '-'
#define TOK_MULTIPLY    '*'
#define TOK_DIVIDE      '/'
#define TOK_LPAREN      '('
#define TOK_RPAREN      ')'

struct Token {
    int type;
    const char* start;
    size_t len;
    int line;
};

void printToken(Token token);

const char* lex(const char* stream, Token* token);

#endif