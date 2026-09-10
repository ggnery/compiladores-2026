/* ast.c - cria e imprime os nós da árvore (Fase 2).
   O significado de cada filho está na tabela do ast.h. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

/* ---------- desenho da arvore ----------

   Uma tela unica de bytes: a linha e' o nivel do no', a coluna e' a posicao
   horizontal. As folhas vao ocupando colunas da esquerda para a direita e
   cada pai fica centrado entre o primeiro e o ultimo filho - por isso
   desenhaNo() so' escreve o pai depois de desenhar os filhos, e devolve a
   coluna onde o centrou.

   Os tracos ficam guardados como os bytes 1..9 e viram UTF-8 so' na hora de
   imprimir: assim cada celula ocupa 1 byte e a conta de coluna e' direta. */

#define NIVEIS   256    /* cada nivel da arvore gasta 2 linhas: rotulo e traco */
#define COLUNAS  8192
#define ESPACO   2      /* colunas em branco entre dois rotulos */

enum { VERT = 1, HORIZ, CANTO_ESQ, CANTO_DIR, DESCE, SOBE, CRUZ, T_ESQ, T_DIR };
static const char* TRACO[] = { "", "│", "─", "┌", "┐", "┬", "┴", "┼", "├", "┤" };

static char tela[NIVEIS][COLUNAS];
static int  proxima;             /* primeira coluna livre para uma folha */
static int  ocupada[NIVEIS];     /* primeira coluna livre de cada linha */
static int  estourou;    /* arvore maior que a tela */

/* Desenha o no' e a subarvore dele; devolve a coluna onde o rotulo ficou centrado. */
static int desenhaNo(No* no, int linha) {
    char rotulo[128];
    int  centro[3], n = 0, i, c, inicio, meio, tam, esq, dir;
    const char* tipo;

    if (linha + 1 >= NIVEIS) {          /* fundo demais: nem desce */
        estourou = 1;
        return proxima < COLUNAS ? proxima : COLUNAS - 1;
    }

    tipo = no->tipo == TIPO_INT ? " : int" : no->tipo == TIPO_CAR ? " : car" : "";
    if (no->lexema)
        snprintf(rotulo, sizeof rotulo, "%s %s%s", nomeEspecie(no->especie), no->lexema, tipo);
    else
        snprintf(rotulo, sizeof rotulo, "%s%s", nomeEspecie(no->especie), tipo);
    tam = (int) strlen(rotulo);

    /* os filhos primeiro: sao eles que dizem onde o pai cabe */
    if (no->filho1) centro[n++] = desenhaNo(no->filho1, linha + 2);
    if (no->filho2) centro[n++] = desenhaNo(no->filho2, linha + 2);
    if (no->filho3) centro[n++] = desenhaNo(no->filho3, linha + 2);

    if (n == 0) {
        inicio = proxima;                          /* folha: primeira coluna livre */
        meio   = inicio + tam / 2;
    } else {
        meio   = (centro[0] + centro[n - 1]) / 2;  /* pai: no meio dos filhos */
        inicio = meio - tam / 2;
        if (inicio < 0) inicio = 0;
    }

    if (inicio < ocupada[linha]) {      /* pai mais largo que os filhos: empurra */
        inicio = ocupada[linha];
        meio   = inicio + tam / 2;
    }

    if (inicio + tam + ESPACO >= COLUNAS) {   /* largo demais */
        estourou = 1;
        return COLUNAS - 1;
    }

    memcpy(tela[linha] + inicio, rotulo, tam);
    ocupada[linha] = inicio + tam + ESPACO;
    if (ocupada[linha] > proxima) proxima = ocupada[linha];

    if (n == 1 && meio == centro[0]) {
        tela[linha + 1][meio] = VERT;   /* filho unico, bem embaixo do pai */
    } else if (n > 0) {
        /* a linha vai do ponto mais a' esquerda ao mais a' direita, contando o pai */
        esq = centro[0] < meio ? centro[0] : meio;
        dir = centro[n - 1] > meio ? centro[n - 1] : meio;
        for (c = esq; c <= dir; c++) tela[linha + 1][c] = HORIZ;

        for (i = 0; i < n; i++)
            tela[linha + 1][centro[i]] = centro[i] == esq ? CANTO_ESQ
                                       : centro[i] == dir ? CANTO_DIR
                                       : DESCE;

        switch (tela[linha + 1][meio]) {           /* o traco que sobe ate' o pai */
            case CANTO_ESQ: tela[linha + 1][meio] = T_ESQ; break;   /* ┌ vira ├ */
            case CANTO_DIR: tela[linha + 1][meio] = T_DIR; break;   /* ┐ vira ┤ */
            case DESCE:     tela[linha + 1][meio] = CRUZ;  break;   /* ┬ vira ┼ */
            default:        tela[linha + 1][meio] = SOBE;  break;
        }
    }
    return meio;
}

void imprimeArvore(No* raiz) {
    int l, c, fim;

    if (!raiz) return;

    memset(tela, ' ', sizeof tela);
    memset(ocupada, 0, sizeof ocupada);
    proxima  = 0;
    estourou = 0;
    desenhaNo(raiz, 0);

    for (l = 0; l < NIVEIS; l++) {
        for (fim = COLUNAS; fim > 0 && tela[l][fim - 1] == ' '; fim--)
            ;                                   /* corta o rabo de espacos */
        if (fim == 0) break;                    /* linha vazia: a arvore acabou */

        for (c = 0; c < fim; c++) {
            unsigned char ch = (unsigned char) tela[l][c];
            if (ch >= VERT && ch <= T_DIR) printf("%s", TRACO[ch]);
            else                           putchar(ch);
        }
        putchar('\n');
    }

    if (estourou) printf("(arvore grande demais para a tela: parte dela nao foi desenhada)\n");
}
