#ifndef _lexer
#define lexer
#include <stdio.h>
#include "tokens.h"
#include "simplequeue.h"

Vector* lexer_lexFile(FILE* file);

#endif