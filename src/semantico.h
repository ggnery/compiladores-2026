#ifndef SEMANTICO_H
#define SEMANTICO_H

/* semantico.h - analisador semântico da linguagem G-V1 (Seção 6 do enunciado)
   Terceira etapa (após léxico e sintático) de fonte -> tokens -> árvore -> semântico -> MIPS.
   O main o chama depois do yyparse; num erro, a compilação para ali, sem gravar a árvore
   nem gerar o .asm. */

#include "ast.h"

/* Percorre a árvore conferindo escopo e tipos, e anota o tipo de cada expressão
   no próprio nó. No primeiro erro, avisa com a linha e encerra a compilação. */
void analisaSemantica(No* raiz);

#endif /* SEMANTICO_H */
