#ifndef CODIGO_H
#define CODIGO_H

/* codigo.h - gerador de código assembly MIPS da linguagem G-V1 */

#include "ast.h"

/* Percorre a árvore (já conferida pelo semântico) e grava o assembly MIPS
   equivalente no arquivo "nomeArquivo", pronto para rodar no SPIM ou no MARS. */
void geraCodigo(No* raiz, const char* nomeArquivo);

#endif /* CODIGO_H */
