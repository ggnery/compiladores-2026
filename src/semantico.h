#ifndef SEMANTICO_H
#define SEMANTICO_H

/* semantico.h - analisador semântico */

#include "ast.h"

/* Confere escopo e tipos e anota o tipo de cada expressão. Para no primeiro erro. */
void analisaSemantica(No* raiz);

#endif /* SEMANTICO_H */
