#ifndef TABELA_H
#define TABELA_H

/* tabela.h - pilha de tabelas de símbolos da linguagem G-V1 (Seção 5 do enunciado).
   Não é uma etapa de fonte -> tokens -> árvore -> semântico -> MIPS: é a estrutura que as duas
   últimas usam enquanto percorrem a árvore. O semântico (Seção 6) pergunta se um nome foi
   declarado e com que tipo; a geração de código (Seção 7), onde a variável está na memória.
   Cada bloco com declarações é um escopo, e os escopos se empilham como os blocos se aninham. */

#include "ast.h"   /* Tipo */

/* Uma variável declarada: uma linha da tabela de símbolos de um escopo. */
typedef struct entrada {
    char*           lexema;         /* nome da variável (ponteiro para o lexema da árvore, sem cópia) */
    Tipo            tipo;           /* int ou car */
    int             linha;          /* linha da declaração */
    int             deslocamento;   /* bytes a partir de $fp (negativo); só a geração de código preenche */
    struct entrada* prox;           /* próxima variável do mesmo escopo */
} Entrada;

/* Um escopo: a tabela de símbolos de um bloco. Os "abaixo" encadeados formam a pilha. */
typedef struct escopo {
    Entrada*       entradas;        /* variáveis deste escopo, a mais recente primeiro */
    struct escopo* abaixo;          /* escopo de fora, mais perto da base */
} Escopo;

/* a) a d): as quatro operações pedidas na Seção 5. */

/* a) Cria a pilha, inicialmente vazia. */
void iniciaPilha(void);

/* b) Cria um escopo vazio e o põe no topo da pilha. */
void empilhaEscopo(void);

/* c) Procura "lexema" do topo para a base; devolve a entrada ou NULL. */
Entrada* busca(char* lexema);

/* d) Remove o escopo do topo e libera suas variáveis. */
void desempilhaEscopo(void);

/* Põe a variável no escopo do topo; devolve NULL se o nome já existe nesse escopo.
   Não está na lista a)-d), mas sem ela nenhuma declaração entra na tabela. */
Entrada* insere(char* lexema, Tipo tipo, int linha);

/* Mostra os escopos, do topo para a base (para a apresentação: make teste-tabela). */
void imprimePilha(void);

#endif /* TABELA_H */
