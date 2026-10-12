#ifndef CODIGO_H
#define CODIGO_H

/* codigo.h - gerador de código assembly MIPS da linguagem G-V1
   Última etapa: fonte -> tokens -> árvore -> semântico -> MIPS (Seção 7 do enunciado).
   Usado pelo main (g-v1.y), depois da análise semântica. */

#include "ast.h"

/* Percorre a árvore (já conferida pelo semântico, com o tipo de cada expressão anotado) e grava
   o assembly MIPS equivalente no arquivo "nomeArquivo", pronto para rodar no SPIM ou no MARS. */
void geraCodigo(No* raiz, const char* nomeArquivo);

#endif /* CODIGO_H */
