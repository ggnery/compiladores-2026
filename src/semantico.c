/* semantico.c - analisador semântico da linguagem G-V1 (Seção 6 do enunciado)
   Terceira etapa (após léxico e sintático) de fonte -> tokens -> árvore -> semântico -> MIPS.
   Entra a árvore do parser; sai a mesma árvore com o tipo de cada expressão anotado em
   no->tipo, ou uma mensagem "ERRO: ... - linha N" e o fim da compilação.
   Confere o que a gramática não consegue, por depender de contexto: nome declarado antes do
   uso e uma vez só por bloco, e int/car sem mistura. Para isso percorre a árvore da raiz para
   as folhas, da esquerda para a direita, com a pilha de tabelas de símbolos do tabela.c (Seção 5). */

#include <stdio.h>
#include <stdlib.h>
#include "semantico.h"
#include "tabela.h"

/* Uma visita por espécie de subárvore: bloco, comando, expressão. Protótipos antes porque elas
   se chamam: bloco aninhado é comando, então visitaBloco e visitaComando chamam uma à outra. */
static void visitaBloco(No* bloco);
static void visitaComando(No* no);
static Tipo visitaExpressao(No* no);

/* Todo erro semântico sai aqui, no formato do erro sintático: ERRO: <mensagem> - linha <n>.
   Nos erros de variável, o nome dela vem antes da mensagem. */
static void erroSemantico(const char* nome, const char* mensagem, int linha) {
    if (nome) printf("ERRO: variavel \"%s\" %s - linha %d\n", nome, mensagem, linha);
    else      printf("ERRO: %s - linha %d\n", mensagem, linha);
    exit(1);                                           /* Seção 6: para no primeiro erro */
}

/* Chamada pelo main logo depois do yyparse. A pilha é iniciada aqui para o percurso
   começar sempre com ela vazia, sem depender de quem chama. */
void analisaSemantica(No* raiz) {
    iniciaPilha();
    visitaBloco(raiz->filho1);   /* PROGRAMA tem um filho só: o bloco principal */
}

/* Declarações à esquerda (filho1), comandos à direita (filho2): visitando a esquerda
   primeiro, todo nome já está na tabela quando algum comando o usa. Bloco aninhado é visitado
   por recursão e desempilha antes de o de fora continuar: a pilha acompanha as chaves. */
static void visitaBloco(No* bloco) {
    No* p;

    if (bloco->filho1) empilhaEscopo();              /* Seção 5: só bloco com declarações abre escopo */

    /* insere só olha o escopo do topo: nome repetido no mesmo bloco é erro (na linha da
       segunda declaração); o mesmo nome de um bloco de fora é sombreamento, permitido. */
    for (p = bloco->filho1; p; p = p->filho2)         /* LISTA_DECL: um DECL por nome */
        if (!insere(p->filho1->lexema, p->filho1->tipo, p->filho1->linha))
            erroSemantico(p->filho1->lexema, "ja declarada neste escopo", p->filho1->linha);

    for (p = bloco->filho2; p; p = p->filho2)         /* LISTA_CMD: um comando por nó */
        visitaComando(p->filho1);

    if (bloco->filho1) desempilhaEscopo();           /* fim do bloco: as variáveis dele saem */
}

/* Não há tipo lógico em G-V1: expressões lógicas e relacionais dão int (Seção 6) e a condição
   de se/enquanto é testada contra 0. Exigir int na condição é interpretação nossa (car seria
   caractere usado como valor lógico); para aceitar car, bastaria tirar este if. */
static void visitaCondicao(No* condicao) {
    if (visitaExpressao(condicao) != TIPO_INT)
        erroSemantico(NULL, "condicao deve ser do tipo int", condicao->linha);
}

/* Comando não tem tipo: cada case só manda os filhos para visitaBloco, visitaCondicao
   ou visitaExpressao, onde ficam as regras. */
static void visitaComando(No* no) {
    if (!no) return;                                   /* comando vazio (;) não gera nó; senão ausente é NULL */

    switch (no->especie) {
    case BLOCO:                                        /* bloco aninhado: escopo novo se tiver declarações */
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
    case LEIA_CMD:                                     /* filho1 é o ID: basta estar declarado */
    case ESCREVA_CMD:                                  /* int ou car, os dois podem ser escritos */
        visitaExpressao(no->filho1);                   /* o tipo anotado escolhe a syscall no codigo.c */
        break;
    case ESCREVA_STR:
    case NOVALINHA_CMD:                                /* nada a conferir */
        break;
    default:                                           /* expressão como comando (x = 1;): Expr ';' não cria nó de comando */
        visitaExpressao(no);
    }
}

/* Confere a expressão, anota o tipo em no->tipo (o arvore.txt e o codigo.c usam) e o devolve.
   Pós-ordem: operandos antes do operador, porque a regra do operador depende do tipo dos
   dois lados. Por isso, em "c = c + z" com c car e z não declarado, o erro acusado é o do z,
   achado na descida, e não o do + com operando car. */
static Tipo visitaExpressao(No* no) {
    Tipo     esq = TIPO_NENHUM, dir = TIPO_NENHUM;   /* tipos de filho1 e filho2 (NENHUM se não há o filho) */
    Entrada* var;

    if (no->filho1) esq = visitaExpressao(no->filho1);
    if (no->filho2) dir = visitaExpressao(no->filho2);

    switch (no->especie) {
    case INT_CONST:                                    /* 42 ou +42 (o '+' não gera nó); -42 é NEG_OP */
        no->tipo = TIPO_INT;
        break;
    case CAR_CONST:
        no->tipo = TIPO_CAR;
        break;
    case ID:                                           /* busca vai do topo à base: vale a declaração mais interna */
        var = busca(no->lexema);
        if (!var) erroSemantico(no->lexema, "nao declarada", no->linha);
        no->tipo = var->tipo;                          /* var não é NULL: erroSemantico não retorna */
        break;
    case ATRIB:                                        /* x = e: esq é o tipo de x, dir o de e; sem conversão int/car */
        if (esq != dir) erroSemantico(NULL, "tipos incompativeis na atribuicao", no->linha);
        no->tipo = esq;                                /* atribuição é expressão: x = y = 1 vale */
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
    case OU_OP: case E_OP:                             /* a || b,  a & b */
        if (esq != TIPO_INT || dir != TIPO_INT) erroSemantico(NULL, "operacao logica exige operandos do tipo int", no->linha);
        no->tipo = TIPO_INT;
        break;
    default:                                           /* relacionais (< > <= >= == !=): mesmo tipo, int ou car */
        if (esq != dir) erroSemantico(NULL, "operacao relacional exige operandos do mesmo tipo", no->linha);
        no->tipo = TIPO_INT;                           /* verdadeiro é 1, falso é 0 */
    }
    return no->tipo;
}
