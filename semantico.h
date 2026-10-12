#ifndef SEMANTICO_H
#define SEMANTICO_H

/* semantico.h - analisador semântico da linguagem G-V1 */

#include "ast.h"

/* Percorre a árvore conferindo escopo e tipos, e anota o tipo de cada expressão
   no próprio nó. No primeiro erro, avisa com a linha e encerra a compilação. */
void analisaSemantica(No* raiz);

#endif /* SEMANTICO_H */
