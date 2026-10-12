/* ast.c - cria os nós e grava a árvore em arquivo */

#include <stdio.h>
#include <stdlib.h>
#include "ast.h"

const char* nomeEspecie(Especie especie) {
    static const char* nomes[] = {   /* mesma ordem do enum Especie */
        "PROGRAMA", "BLOCO", "LISTA_DECL", "DECL", "LISTA_CMD",
        "LEIA_CMD", "ESCREVA_CMD", "ESCREVA_STR", "NOVALINHA_CMD", "SE_CMD", "ENQUANTO_CMD",
        "ATRIB", "OU_OP", "E_OP", "IGUAL_OP", "DIFERENTE_OP",
        "MENOR_OP", "MAIOR_OP", "MAIORIGUAL_OP", "MENORIGUAL_OP",
        "MAIS_OP", "MENOS_OP", "MULT_OP", "DIV_OP",
        "NEG_OP", "NAO_OP",
        "ID", "INT_CONST", "CAR_CONST", "STR_CONST"
    };
    return nomes[especie];   /* a espécie é o próprio índice */
}

No* criaNo(Especie especie, int linha, char* lexema, No* f1, No* f2, No* f3) {
    No* no = malloc(sizeof(No));
    if (!no) {
        printf("ERRO: memoria insuficiente\n");
        exit(1);
    }
    
    no->especie = especie;
    no->linha   = linha;
    no->lexema  = lexema;          /* o léxico já fez a cópia */
    no->tipo    = TIPO_NENHUM;
    no->filho1  = f1;
    no->filho2  = f2;
    no->filho3  = f3;
    return no;
}

/* Em "a, b, c : int;", nome é o a e outros é a lista de b e c. */
No* declara(char* nome, int linha, No* outros, Tipo tipo, No* resto) {
    No* lista = criaNo(LISTA_DECL, linha, NULL,
                       criaNo(DECL, linha, nome, NULL, NULL, NULL), outros, NULL);   /* primeiro nome + os outros */
    No* p;

    for (p = lista; p; p = p->filho2) p->filho1->tipo = tipo;   /* carimba o tipo em cada DECL */
    for (p = lista; p->filho2; p = p->filho2) ;                 /* anda até o fim da lista */
    p->filho2 = resto;                                          /* emenda as linhas seguintes (já têm tipo) */
    return lista;
}

/* Grava o nó e depois os filhos (pré-ordem), em galhos:

   PROGRAMA  (linha 1)
   └── BLOCO  (linha 2)
       ├── LISTA_DECL  (linha 2)
       │   └── DECL x : int  (linha 2)
       └── LISTA_CMD  (linha 4)
*/
static void imprimeNo(FILE* saida, No* no, const char* prefixo, const char* galho, const char* recuo) {
    No*  filhos[3] = { no->filho1, no->filho2, no->filho3 };
    char proximo[1024];
    int  i, ultimo = -1;

    fprintf(saida, "%s%s%s", prefixo, galho, nomeEspecie(no->especie));
    if (no->lexema)              fprintf(saida, " %s", no->lexema);
    if (no->tipo != TIPO_NENHUM) fprintf(saida, " : %s", no->tipo == TIPO_INT ? "int" : "car");
    fprintf(saida, "  (linha %d)\n", no->linha);

    snprintf(proximo, sizeof proximo, "%s%s", prefixo, recuo);   /* prefixo dos filhos */

    for (i = 0; i < 3; i++)             /* acha o último filho */
        if (filhos[i]) ultimo = i;

    for (i = 0; i < 3; i++) {
        if (!filhos[i]) {
            if (i < ultimo)             /* NULL no meio vira (vazio): posições não mudam */
                fprintf(saida, "%s├── (vazio)\n", proximo);
            continue;
        }
        if (i == ultimo) imprimeNo(saida, filhos[i], proximo, "└── ", "    ");   /* último: coluna fica em branco */
        else             imprimeNo(saida, filhos[i], proximo, "├── ", "│   ");   /* vem irmão: coluna continua com │ */
    }
}

void imprimeArvore(No* raiz, const char* nomeArquivo) {
    FILE* saida = fopen(nomeArquivo, "w");   /* cria o arquivo, ou apaga o anterior */
    if (!saida) {
        printf("Não foi possível criar o arquivo %s\n", nomeArquivo);
        exit(1);
    }
    if (raiz) imprimeNo(saida, raiz, "", "", "");   /* a raiz não tem galho */
    fclose(saida);
}
