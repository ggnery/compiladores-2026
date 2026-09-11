/* ast.c - cria e imprime os nós da árvore (Fase 2).
   O significado de cada filho está na tabela do ast.h. */

#include <stdio.h>
#include <stdlib.h>
#include "ast.h"

const char* nomeEspecie(Especie especie) {
    switch (especie) {
        case PROGRAMA:      return "PROGRAMA";
        case BLOCO:         return "BLOCO";
        case LISTA_DECL:    return "LISTA_DECL";
        case DECL:          return "DECL";
        case LISTA_CMD:     return "LISTA_CMD";
        case LEIA_CMD:      return "LEIA_CMD";
        case ESCREVA_CMD:   return "ESCREVA_CMD";
        case ESCREVA_STR:   return "ESCREVA_STR";
        case NOVALINHA_CMD: return "NOVALINHA_CMD";
        case SE_CMD:        return "SE_CMD";
        case ENQUANTO_CMD:  return "ENQUANTO_CMD";
        case ATRIB:         return "ATRIB";
        case OU_OP:         return "OU_OP";
        case E_OP:          return "E_OP";
        case IGUAL_OP:      return "IGUAL_OP";
        case DIFERENTE_OP:  return "DIFERENTE_OP";
        case MENOR_OP:      return "MENOR_OP";
        case MAIOR_OP:      return "MAIOR_OP";
        case MAIORIGUAL_OP: return "MAIORIGUAL_OP";
        case MENORIGUAL_OP: return "MENORIGUAL_OP";
        case MAIS_OP:       return "MAIS_OP";
        case MENOS_OP:      return "MENOS_OP";
        case MULT_OP:       return "MULT_OP";
        case DIV_OP:        return "DIV_OP";
        case NEG_OP:        return "NEG_OP";
        case NAO_OP:        return "NAO_OP";
        case ID:            return "ID";
        case INT_CONST:     return "INT_CONST";
        case CAR_CONST:     return "CAR_CONST";
        case STR_CONST:     return "STR_CONST";
    }
    return "???";   /* sem "default": assim o -Wall avisa se faltar uma espécie */
}

No* criaNo(Especie especie, int linha, char* lexema, No* f1, No* f2, No* f3) {
    No* no = malloc(sizeof(No));
    if (!no) {
        printf("ERRO: memoria insuficiente\n");
        exit(1);
    }
    /* malloc não zera: todo campo precisa ser escrito aqui. */
    no->especie = especie;
    no->linha   = linha;
    no->lexema  = lexema;   /* já é cópia; NULL quando não há texto */
    no->tipo    = TIPO_NENHUM;
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

   Cada nó imprime o prefixo herdado dos pais, o seu galho e o rótulo.
   O último filho usa └── e passa espaços aos netos; os outros usam ├──
   e passam │, que continua a linha vertical. */

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
