#include "lexer.h"

void printToken(Token token) {
    if (token.type == 0)  printf("EOF");
    else                  printf("%lu|%d\t%.*s", token.line, token.type, token.len, token.start);
}

static int checkKeyword(const char* start, const char* end, const char* keyword) {
    size_t len = strlen(keyword);
    return (end - start) == len && strncmp(start, keyword, strlen(keyword)) == 0;
}

static int identifierType(const char* start, const char* end) {
    if (checkKeyword(start, end, "const"))      return TOK_CONST;
    if (checkKeyword(start, end, "var"))        return TOK_VAR;
    if (checkKeyword(start, end, "procedure"))  return TOK_PROCEDURE;
    if (checkKeyword(start, end, "call"))       return TOK_CALL;
    if (checkKeyword(start, end, "begin"))      return TOK_BEGIN;
    if (checkKeyword(start, end, "end"))        return TOK_END;
    if (checkKeyword(start, end, "if"))         return TOK_IF;
    if (checkKeyword(start, end, "then"))       return TOK_THEN;
    if (checkKeyword(start, end, "while"))      return TOK_WHILE;
    if (checkKeyword(start, end, "do"))         return TOK_DO;
    if (checkKeyword(start, end, "odd"))        return TOK_ODD;

    return TOK_IDENT;
}

static const char* skipWhitespaces(const char* cursor, int* line) {
    while (*cursor != '\0') {
        switch (*cursor)
        {
        case '\n':                      ++(*line);
        case ' ': case '\t': case '\r': ++cursor; break;
        case '{':   /* skip comments */
            while (*cursor != '}') {
                if (*cursor == '\0') errorAt(*line, "unterminated comment");
                if (*cursor == '\n') ++(*line);
                ++cursor;
            }
            ++cursor;
            break;
        default:
            return cursor;
        }
    }
    return cursor;
}

const char* lex(const char* stream, Token* token) {
    int line = 0;
    const char* cursor = skipWhitespaces(stream, &line);

    token->type = -1;
    token->line += line;
    token->start = cursor;

    if (*cursor == '\0')
    {
        token->type = 0;
        return cursor;
    }

    switch (*(cursor++)) {
    /* scan number */
    case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
    {
        while (isdigit(*cursor)) ++cursor;
        token->type = TOK_NUMBER;
        break;
    }
    /* scan identifiers and keywords */
    case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': case 'h': case 'i': case 'j':
    case 'k': case 'l': case 'm': case 'n': case 'o': case 'p': case 'q': case 'r': case 's': case 't':
    case 'u': case 'v': case 'w': case 'x': case 'y': case 'z':
    case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G': case 'H': case 'I': case 'J':
    case 'K': case 'L': case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T':
    case 'U': case 'V': case 'W': case 'X': case 'Y': case 'Z':
    case '_':
    {
        while (isalnum(*cursor) || *cursor == '_') ++cursor;
        token->type = identifierType(token->start, cursor);
        break;
    }
    case '.':
    case '=':
    case ',':
    case ';':
    case '#':
    case '<':
    case '>':
    case '+':
    case '-':
    case '*':
    case '/':
    case '(':
    case ')':
        token->type = *token->start;
        break;
    case ':':
        if (*(cursor++) == '=') {
            token->type = TOK_ASSIGN;
        }
        break;
    }

    if (token->type < 0) {
        errorAt(token->line, "unknown token: '%c'", *token->start);
    }

    token->len = cursor - token->start;
    return cursor;
}