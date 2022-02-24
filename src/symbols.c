#include "symbols.h"

symbol* symbolsInit() {
    symbol* head = malloc(sizeof(symbol));
    if (!head) error("malloc failed");

    head->type = TOK_PROCEDURE;
    head->name = "main";
    head->depth = 0;
    head->next = NULL;

    return head;
}

static int symbolsCheckName(const char* name, const Token* token) {
    return strlen(name) == token->len && strncmp(name, token->start, token->len) == 0;
}

void symbolsAdd(symbol* head, Token* token, int type, int depth) {
    symbol* current = head;

    while (1) {
        if (symbolsCheckName(current->name, token) && current->depth == (depth - 1)) {
            errorAt(token->line, "duplicate symbol: %.*s", token->len, token->start);
        }

        if (!current->next) break;

        current = current->next;
    }

    symbol* new = malloc(sizeof(symbol));
    if (!new) error("malloc failed");

    new->name = malloc(token->len + 1);
    if (!new->name) error("malloc failed");

    memcpy(new->name, token->start, token->len);
    new->name[token->len] = '\0';
    new->depth = depth - 1;
    new->type = type;
    new->next = NULL;

    printf("new symbol: %s\n", new->name);

    current->next = new;
}

void symbolsDestroy(symbol* head) {
    struct symbol *curr, *prev;

again:
    curr = head;
    while (curr->next != NULL) {
        prev = curr;
        curr = curr->next;
    }

    if (curr->type != TOK_PROCEDURE) {
        free(curr->name);
        free(curr);
        prev->next = NULL;
        goto again;
    }
}


void symbolsCheck(symbol* head, const Token* token, int check) {
    symbol* sym = head;
    while (sym != NULL) {
        if (symbolsCheckName(sym->name, token)) break;
        sym = sym->next;
    }

    if (sym == NULL)
        errorAt(token->line, "undefined symbol: %.*s", token->len, token->start);

    switch (check) {
    case CHECK_LHS:
        if (sym->type != TOK_VAR)
            errorAt(token->line, "must be a variable:  %.*s", token->len, token->start);
        break;
    case CHECK_RHS:
        if (sym->type == TOK_PROCEDURE)
            errorAt(token->line, "must not be a procedure:  %.*s", token->len, token->start);
        break;
    case CHECK_CALL:
        if (sym->type != TOK_PROCEDURE)
            errorAt(token->line, "must be a procedure:  %.*s", token->len, token->start);
    }
}