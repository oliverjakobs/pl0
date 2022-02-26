#ifndef PL0_CODE_GENERATOR_H
#define PL0_CODE_GENERATOR_H

#include "common.h"

void cgInit();

void cgConst(const Token* token);
void cgVar(const Token* token);
void cgCall(const Token* token);

void cgSymbol(const Token* token);

void cgProcedure(int proc, const Token* token);
void cgEpilogue(int proc);

void cgOdd();

void cgWriteChar(const Token* token);
void cgWriteInt(const Token* token);

void cgSemicolon();
void cgEnd();

#endif