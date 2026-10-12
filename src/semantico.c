/* semantico.c - analisador semântico: confere escopo e tipos percorrendo a árvore */

#include <stdio.h>
#include <stdlib.h>
#include "semantico.h"
#include "tabela.h"

static void visitaBloco(No* bloco);
static void visitaComando(No* no);
static Tipo visitaExpressao(No* no);

/* Mostra o erro com a linha e encerra a compilação. */
static void erroSemantico(const char* nome, const char* mensagem, int linha) {
    if (nome) printf("ERRO: variavel \"%s\" %s - linha %d\n", nome, mensagem, linha);
    else      printf("ERRO: %s - linha %d\n", mensagem, linha);
    exit(1);
}

void analisaSemantica(No* raiz) {
    iniciaPilha();
    visitaBloco(raiz->filho1);   /* o bloco principal */
}

/* Declarações (filho1) antes dos comandos (filho2): todo nome já está na tabela quando é usado. */
static void visitaBloco(No* bloco) {
    No* p;

    if (bloco->filho1) empilhaEscopo();               /* só bloco com declarações abre escopo */

    for (p = bloco->filho1; p; p = p->filho2)         /* cada declaração */
        if (!insere(p->filho1->lexema, p->filho1->tipo, p->filho1->linha))
            erroSemantico(p->filho1->lexema, "ja declarada neste escopo", p->filho1->linha);

    for (p = bloco->filho2; p; p = p->filho2)         /* cada comando */
        visitaComando(p->filho1);

    if (bloco->filho1) desempilhaEscopo();           /* as variáveis do bloco saem */
}

/* A condição de se/enquanto tem de ser int (0 é falso). */
static void visitaCondicao(No* condicao) {
    if (visitaExpressao(condicao) != TIPO_INT)
        erroSemantico(NULL, "condicao deve ser do tipo int", condicao->linha);
}

static void visitaComando(No* no) {
    if (!no) return;                                   /* comando vazio (;) */

    switch (no->especie) {
    case BLOCO:                                        /* bloco dentro de bloco */
        visitaBloco(no);
        break;
    case SE_CMD:
        visitaCondicao(no->filho1);
        visitaComando(no->filho2);                     /* então */
        visitaComando(no->filho3);                     /* senão */
        break;
    case ENQUANTO_CMD:
        visitaCondicao(no->filho1);
        visitaComando(no->filho2);                     /* corpo */
        break;
    case LEIA_CMD:
    case ESCREVA_CMD:
        visitaExpressao(no->filho1);
        break;
    case ESCREVA_STR:
    case NOVALINHA_CMD:                                /* nada a conferir */
        break;
    default:                                           /* expressão como comando: x = 1; */
        visitaExpressao(no);
    }
}

/* Confere a expressão, anota o tipo no nó e devolve esse tipo.
   Pós-ordem: primeiro os operandos, depois o operador. */
static Tipo visitaExpressao(No* no) {
    Tipo     esq = TIPO_NENHUM, dir = TIPO_NENHUM;
    Entrada* var;

    if (no->filho1) esq = visitaExpressao(no->filho1);
    if (no->filho2) dir = visitaExpressao(no->filho2);

    switch (no->especie) {
    case INT_CONST:
        no->tipo = TIPO_INT;
        break;
    case CAR_CONST:
        no->tipo = TIPO_CAR;
        break;
    case ID:                                           /* o tipo vem da declaração mais interna */
        var = busca(no->lexema);
        if (!var) erroSemantico(no->lexema, "nao declarada", no->linha);
        no->tipo = var->tipo;
        break;
    case ATRIB:                                        /* x = e: mesmo tipo dos dois lados */
        if (esq != dir) erroSemantico(NULL, "tipos incompativeis na atribuicao", no->linha);
        no->tipo = esq;
        break;
    case NEG_OP:                                       /* -a */
        if (esq != TIPO_INT) erroSemantico(NULL, "operacao aritmetica exige operandos do tipo int", no->linha);
        no->tipo = TIPO_INT;
        break;
    case MAIS_OP: case MENOS_OP: case MULT_OP: case DIV_OP:
        if (esq != TIPO_INT || dir != TIPO_INT) erroSemantico(NULL, "operacao aritmetica exige operandos do tipo int", no->linha);
        no->tipo = TIPO_INT;
        break;
    case NAO_OP:                                       /* !a */
        if (esq != TIPO_INT) erroSemantico(NULL, "operacao logica exige operandos do tipo int", no->linha);
        no->tipo = TIPO_INT;
        break;
    case OU_OP: case E_OP:                             /* a || b, a & b */
        if (esq != TIPO_INT || dir != TIPO_INT) erroSemantico(NULL, "operacao logica exige operandos do tipo int", no->linha);
        no->tipo = TIPO_INT;
        break;
    default:                                           /* relacionais: < > <= >= == != */
        if (esq != dir) erroSemantico(NULL, "operacao relacional exige operandos do mesmo tipo", no->linha);
        no->tipo = TIPO_INT;                           /* resultado 1 ou 0 */
    }
    return no->tipo;
}
