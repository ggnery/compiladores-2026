/* semantico.c - analisador semântico da linguagem G-V1 */

#include <stdio.h>
#include <stdlib.h>
#include "semantico.h"
#include "tabela.h"

static void visitaBloco(No* bloco);
static void visitaComando(No* no);
static Tipo visitaExpressao(No* no);

/* Formato de todo erro semântico: ERRO: <mensagem> - linha <n>.
   Nos erros de variável, o nome dela vem antes da mensagem. */
static void erroSemantico(const char* nome, const char* mensagem, int linha) {
    if (nome) printf("ERRO: variavel \"%s\" %s - linha %d\n", nome, mensagem, linha);
    else      printf("ERRO: %s - linha %d\n", mensagem, linha);
    exit(1);
}

void analisaSemantica(No* raiz) {
    iniciaPilha();
    visitaBloco(raiz->filho1);   /* PROGRAMA tem um filho só: o bloco principal */
}

/* Declarações à esquerda (filho1), comandos à direita (filho2): visitando a esquerda
   primeiro, todo nome já está na tabela quando algum comando o usa. */
static void visitaBloco(No* bloco) {
    No* p;

    if (bloco->filho1) empilhaEscopo();              /* só bloco com declarações abre escopo */

    for (p = bloco->filho1; p; p = p->filho2)         /* LISTA_DECL: um DECL por nome */
        if (!insere(p->filho1->lexema, p->filho1->tipo, p->filho1->linha))
            erroSemantico(p->filho1->lexema, "ja declarada neste escopo", p->filho1->linha);

    for (p = bloco->filho2; p; p = p->filho2)         /* LISTA_CMD: um comando por nó */
        visitaComando(p->filho1);

    if (bloco->filho1) desempilhaEscopo();           /* fim do bloco: as variáveis dele saem */
}

/* Não há tipo lógico em G-V1: a condição de se/enquanto é um int (0 = falso). */
static void visitaCondicao(No* condicao) {
    if (visitaExpressao(condicao) != TIPO_INT)
        erroSemantico(NULL, "condicao deve ser do tipo int", condicao->linha);
}

static void visitaComando(No* no) {
    if (!no) return;                                   /* comando vazio (;) não gera nó */

    switch (no->especie) {
    case BLOCO:                                        /* bloco aninhado: escopo novo */
        visitaBloco(no);
        break;
    case SE_CMD:
        visitaCondicao(no->filho1);
        visitaComando(no->filho2);                     /* então */
        visitaComando(no->filho3);                     /* senão (NULL se não há) */
        break;
    case ENQUANTO_CMD:
        visitaCondicao(no->filho1);
        visitaComando(no->filho2);                     /* corpo */
        break;
    case LEIA_CMD:                                     /* a variável tem de estar declarada */
    case ESCREVA_CMD:                                  /* int ou car, os dois podem ser escritos */
        visitaExpressao(no->filho1);
        break;
    case ESCREVA_STR:
    case NOVALINHA_CMD:                                /* nada a conferir */
        break;
    default:                                           /* expressão usada como comando: x = 1; */
        visitaExpressao(no);
    }
}

/* Confere a expressão, anota o tipo dela no nó e devolve esse tipo.
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
    case ID:                                           /* o tipo vem da declaração mais próxima */
        var = busca(no->lexema);
        if (!var) erroSemantico(no->lexema, "nao declarada", no->linha);
        no->tipo = var->tipo;
        break;
    case ATRIB:                                        /* x = e: sem conversão entre int e car */
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
    case OU_OP: case E_OP:
        if (esq != TIPO_INT || dir != TIPO_INT) erroSemantico(NULL, "operacao logica exige operandos do tipo int", no->linha);
        no->tipo = TIPO_INT;
        break;
    default:                                           /* relacionais: < > <= >= == != */
        if (esq != dir) erroSemantico(NULL, "operacao relacional exige operandos do mesmo tipo", no->linha);
        no->tipo = TIPO_INT;                           /* verdadeiro é 1, falso é 0 */
    }
    return no->tipo;
}
