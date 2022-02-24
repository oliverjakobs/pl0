#ifndef PL0_COMMON_H
#define PL0_COMMON_H

#include <ctype.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

#define PL0C_VERSION    "1.0.0"

typedef struct Token Token;

void error(const char* fmt, ...);
void errorAt(int line, const char* fmt, ...);

char* readFile(const char* path);

#endif