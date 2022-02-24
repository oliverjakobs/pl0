#ifndef PL0_CODE_GENERATOR_H
#define PL0_CODE_GENERATOR_H

#include "common.h"

void cg_const(const Token* token);
void cg_var(const Token* token);
void cg_call(const Token* token);

void cg_symbol(const Token* token);

void cg_procedure(int proc, const Token* token);
void cg_epilogue(int proc);

void cg_odd();

void cg_semicolon();
void cg_end();

#endif