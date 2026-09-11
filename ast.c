/* ast.c - cria e imprime os nós da árvore */

#include <stdio.h>
#include <stdlib.h>
#include "ast.h"

/* retorna a string da espécie de um nó */
const char* nomeEspecie(Especie especie) {
    static const char* nomes[] = {   /* mesma ordem do enum Especie, no ast.h */
        "PROGRAMA", "BLOCO", "LISTA_DECL", "DECL", "LISTA_CMD",
        "LEIA_CMD", "ESCREVA_CMD", "ESCREVA_STR", "NOVALINHA_CMD", "SE_CMD", "ENQUANTO_CMD",
        "ATRIB", "OU_OP", "E_OP", "IGUAL_OP", "DIFERENTE_OP",
        "MENOR_OP", "MAIOR_OP", "MAIORIGUAL_OP", "MENORIGUAL_OP",
        "MAIS_OP", "MENOS_OP", "MULT_OP", "DIV_OP",
        "NEG_OP", "NAO_OP",
        "ID", "INT_CONST", "CAR_CONST", "STR_CONST"
    };
    return nomes[especie];   /* o enum começa em 0, então a espécie é o próprio índice */
}

No* criaNo(Especie especie, int linha, char* lexema, No* f1, No* f2, No* f3) {
    No* no = malloc(sizeof(No));
    if (!no) {
        printf("ERRO: memoria insuficiente\n");
        exit(1);
    }
    
    no->especie = especie;
    no->linha   = linha;
    no->lexema  = lexema;
    no->tipo    = TIPO_NENHUM;   /* ainda sem tipo: o DECL recebe na declara, as expressões na Fase 4 */
    no->filho1  = f1;
    no->filho2  = f2;
    no->filho3  = f3;
    return no;
}

/* Uma linha "a, b, c : int;" vira três DECL numa LISTA_DECL. O tipo só é
   conhecido depois dos nomes, então ele é carimbado aqui, no fim; "resto" é
   emendado só depois, para não carimbar as declarações das linhas seguintes. */
No* declara(char* nome, int linha, No* outros, Tipo tipo, No* resto) {
    No* lista = criaNo(LISTA_DECL, linha, NULL,
                       criaNo(DECL, linha, nome, NULL, NULL, NULL), outros, NULL);   /* primeiro nome + os outros */
    No* p;

    for (p = lista; p; p = p->filho2) p->filho1->tipo = tipo;   /* carimba o tipo em cada DECL */
    for (p = lista; p->filho2; p = p->filho2) ;                 /* anda até o fim da lista */
    p->filho2 = resto;                                          /* emenda as linhas seguintes */
    return lista;
}

/* ---------- impressão da árvore ----------

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
    fprintf(saida, "  (linha %d)\n", no->linha);   /* a linha do fonte guardada no nó */

    snprintf(proximo, sizeof proximo, "%s%s", prefixo, recuo);   /* prefixo dos filhos */

    for (i = 0; i < 3; i++)             /* acha o último filho que existe */
        if (filhos[i]) ultimo = i;

    for (i = 0; i < 3; i++) {
        if (!filhos[i]) continue;       /* ausência é NULL: pula */
        if (i == ultimo) imprimeNo(saida, filhos[i], proximo, "└── ", "    ");
        else             imprimeNo(saida, filhos[i], proximo, "├── ", "│   ");
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
