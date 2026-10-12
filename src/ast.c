/* ast.c - construção e gravação da árvore sintática abstrata (Seção 4 do enunciado), a etapa
   "árvore" de fonte -> tokens -> árvore -> semântico -> MIPS.
   Entra: o que as ações do g-v1.y passam a criaNo e declara durante o parse; quando o yyparse()
   termina, a árvore está pronta na memória para o semântico e o gerador de MIPS.
   Sai: build/arvore.txt (imprimeArvore), para conferir a forma da árvore. */

#include <stdio.h>
#include <stdlib.h>
#include "ast.h"

/* Texto de cada espécie, para gravar a árvore legível. */
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

/* Todo nó nasce aqui (declara também usa esta função). O tipo começa TIPO_NENHUM e é
   preenchido depois por declara (DECL) ou pelo semântico (expressões). */
No* criaNo(Especie especie, int linha, char* lexema, No* f1, No* f2, No* f3) {
    No* no = malloc(sizeof(No));
    if (!no) {
        printf("ERRO: memoria insuficiente\n");
        exit(1);
    }
    
    no->especie = especie;
    no->linha   = linha;
    no->lexema  = lexema;          /* sem nova cópia: o léxico já entregou uma (copia, no g-v1.l) */
    no->tipo    = TIPO_NENHUM;
    no->filho1  = f1;
    no->filho2  = f2;
    no->filho3  = f3;
    return no;
}

/* Uma linha "a, b, c : int;" vira três DECL numa LISTA_DECL ("nome" é o a; "outros", a lista
   que o DeclVar já montou para ", b, c"). O tipo só aparece depois dos nomes, então é carimbado
   aqui, no fim; "resto" (as linhas seguintes, que já têm o tipo delas) só é emendado depois,
   para não ser carimbado também. */
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
   Recursiva, em pré-ordem: o nó, depois filho1, filho2, filho3. Cada linha é o prefixo herdado
   do pai + o galho deste nó + o rótulo; "recuo" é o que este nó acrescenta ao prefixo dos filhos.

   PROGRAMA  (linha 1)
   └── BLOCO  (linha 2)
       ├── LISTA_DECL  (linha 2)
       │   └── DECL x : int  (linha 2)
       └── LISTA_CMD  (linha 4)
*/
static void imprimeNo(FILE* saida, No* no, const char* prefixo, const char* galho, const char* recuo) {
    No*  filhos[3] = { no->filho1, no->filho2, no->filho3 };
    char proximo[1024];   /* passa de 1024 só com centenas de níveis (cada comando de uma lista desce um); aí o snprintf trunca, não estoura */
    int  i, ultimo = -1;

    fprintf(saida, "%s%s%s", prefixo, galho, nomeEspecie(no->especie));
    if (no->lexema)              fprintf(saida, " %s", no->lexema);
    if (no->tipo != TIPO_NENHUM) fprintf(saida, " : %s", no->tipo == TIPO_INT ? "int" : "car");
    fprintf(saida, "  (linha %d)\n", no->linha);   /* a linha do fonte guardada no nó */

    snprintf(proximo, sizeof proximo, "%s%s", prefixo, recuo);   /* prefixo dos filhos */

    for (i = 0; i < 3; i++)             /* acha o último filho que existe: só ele leva └── */
        if (filhos[i]) ultimo = i;

    /* NULL depois do último filho não aparece; antes de um filho existente vira "(vazio)", para a
       posição não mudar de sentido: "se (c) entao ; senao X" não pode parecer "entao X", e um
       BLOCO só com comandos mostra (vazio) no lugar das declarações. */
    for (i = 0; i < 3; i++) {
        if (!filhos[i]) {
            if (i < ultimo)
                fprintf(saida, "%s├── (vazio)\n", proximo);
            continue;
        }
        if (i == ultimo) imprimeNo(saida, filhos[i], proximo, "└── ", "    ");   /* último: abaixo dele, a coluna fica em branco */
        else             imprimeNo(saida, filhos[i], proximo, "├── ", "│   ");   /* vem irmão abaixo: a coluna continua com │ */
    }
}

/* O main só chama se o semântico passou, por isso a árvore gravada já mostra os tipos das expressões. */
void imprimeArvore(No* raiz, const char* nomeArquivo) {
    FILE* saida = fopen(nomeArquivo, "w");   /* cria o arquivo, ou apaga o anterior */
    if (!saida) {
        printf("Não foi possível criar o arquivo %s\n", nomeArquivo);
        exit(1);
    }
    if (raiz) imprimeNo(saida, raiz, "", "", "");   /* a raiz não tem galho */
    fclose(saida);
}
