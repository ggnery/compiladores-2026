/* tabela.c - pilha de tabelas de símbolos da linguagem G-V1 (Seção 5 do enunciado).
   Implementa o tabela.h para o semântico e a geração de código. São duas listas encadeadas:
   os escopos, ligados por "abaixo", formam a pilha; as variáveis de um escopo, por "prox".
   Lista e não hash: um bloco tem poucas variáveis; trocar a estrutura só mexeria neste arquivo. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tabela.h"

static Escopo* topo;   /* escopo do bloco atual; NULL = pilha vazia. static: só as funções deste arquivo mexem nela */

/* a) Chamada no começo de cada percurso (semântico e geração de código): cada um monta a sua pilha. */
void iniciaPilha(void) {
    topo = NULL;
}

/* b) Ao entrar num bloco com declarações: o escopo novo vira o topo. */
void empilhaEscopo(void) {
    Escopo* novo = malloc(sizeof(Escopo));
    if (!novo) { printf("ERRO: memoria insuficiente\n"); exit(1); }

    novo->entradas = NULL;   /* começa sem variáveis */
    novo->abaixo   = topo;   /* o escopo atual fica embaixo do novo */
    topo = novo;
}

/* c) O topo é o bloco mais interno, e a declaração mais próxima do uso é a que vale:
      é assim que o x de dentro esconde o x de fora sem que isso seja erro. */
Entrada* busca(char* lexema) {
    Escopo*  s;
    Entrada* e;

    for (s = topo; s; s = s->abaixo)             /* do topo para a base */
        for (e = s->entradas; e; e = e->prox)    /* variáveis de um escopo */
            if (strcmp(e->lexema, lexema) == 0)
                return e;                         /* o primeiro achado vence */
    return NULL;
}

/* d) Ao sair do bloco: as variáveis dele deixam de existir e o escopo de fora volta a ser o topo. */
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

/* Só o topo é checado: repetir o nome no mesmo bloco é erro; num bloco de dentro é permitido.
   Devolve a entrada para a geração de código gravar nela o deslocamento. */
Entrada* insere(char* lexema, Tipo tipo, int linha) {
    Entrada* e;

    if (!topo) { printf("ERRO: insere sem escopo aberto\n"); exit(1); }   /* bug do compilador, não do programa G-V1 */

    for (e = topo->entradas; e; e = e->prox)     /* só o escopo do topo conta */
        if (strcmp(e->lexema, lexema) == 0)
            return NULL;                          /* redeclaração no mesmo escopo */

    e = malloc(sizeof(Entrada));
    if (!e) { printf("ERRO: memoria insuficiente\n"); exit(1); }

    e->lexema       = lexema;
    e->tipo         = tipo;
    e->linha        = linha;
    e->deslocamento = 0;                          /* definido só na geração de código */
    e->prox         = topo->entradas;             /* entra na cabeça da lista: ligar é um passo só */
    topo->entradas  = e;
    return e;
}

/* Para a demonstração (make teste-tabela): numera os escopos da base (1) ao topo (n)
   e os mostra do topo para a base, cada um com a variável mais recente primeiro. */
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
