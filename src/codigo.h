#ifndef CODIGO_H
#define CODIGO_H

/* codigo.h - gerador de código MIPS */

#include "ast.h"

/* Grava no arquivo nomeArquivo o assembly MIPS da árvore. */
void geraCodigo(No* raiz, const char* nomeArquivo);

#endif /* CODIGO_H */
