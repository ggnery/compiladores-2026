/* tabela.c - pilha de tabelas de símbolos da linguagem G-V1 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tabela.h"

static Escopo* topo;   /* escopo do bloco atual; NULL = pilha vazia */

/* a) */
void iniciaPilha(void) {
    topo = NULL;
}

/* b) */
void empilhaEscopo(void) {
    Escopo* novo = malloc(sizeof(Escopo));
    if (!novo) { printf("ERRO: memoria insuficiente\n"); exit(1); }

    novo->entradas = NULL;   /* começa sem variáveis */
    novo->abaixo   = topo;   /* o escopo atual fica embaixo do novo */
    topo = novo;
}

/* c) */
Entrada* busca(char* lexema) {
    Escopo*  s;
    Entrada* e;

    for (s = topo; s; s = s->abaixo)             /* do topo para a base */
        for (e = s->entradas; e; e = e->prox)    /* variáveis de um escopo */
            if (strcmp(e->lexema, lexema) == 0)
                return e;                         /* o primeiro achado vence */
    return NULL;
}

/* d) */
void desempilhaEscopo(void) {
    Escopo*  velho = topo;
    Entrada* e, *prox;

    if (!velho) return;                          /* pilha vazia: nada a remover */

    for (e = velho->entradas; e; e = prox) {     /* libera as variáveis do escopo */
        prox = e->prox;
        free(e);                                  /* o lexema é da árvore: não se libera aqui */
    }
    topo = velho->abaixo;
    free(velho);
}

Entrada* insere(char* lexema, Tipo tipo, int linha) {
    Entrada* e;

    if (!topo) { printf("ERRO: insere sem escopo aberto\n"); exit(1); }

    for (e = topo->entradas; e; e = e->prox)     /* só o escopo do topo conta */
        if (strcmp(e->lexema, lexema) == 0)
            return NULL;                          /* redeclaração no mesmo escopo */

    e = malloc(sizeof(Entrada));
    if (!e) { printf("ERRO: memoria insuficiente\n"); exit(1); }

    e->lexema       = lexema;
    e->tipo         = tipo;
    e->linha        = linha;
    e->deslocamento = 0;                          /* definido só na geração de código */
    e->prox         = topo->entradas;             /* entra na cabeça da lista */
    topo->entradas  = e;
    return e;
}

void imprimePilha(void) {
    Escopo*  s;
    Entrada* e;
    int      n = 0;

    for (s = topo; s; s = s->abaixo) n++;         /* quantos escopos há */

    printf("--- pilha de escopos (topo primeiro) ---\n");
    if (n == 0) printf("(vazia)\n");
    for (s = topo; s; s = s->abaixo, n--) {       /* o topo é o de número maior */
        printf("escopo %d:", n);
        for (e = s->entradas; e; e = e->prox)
            printf("  %s (%s, linha %d)", e->lexema, e->tipo == TIPO_INT ? "int" : "car", e->linha);
        printf("\n");
    }
}
