#ifndef TABELA_H
#define TABELA_H

/* tabela.h - pilha de tabelas de símbolos da linguagem G-V1 */

#include "ast.h"   /* Tipo */

/* Uma variável declarada. */
typedef struct entrada {
    char*           lexema;         /* nome da variável */
    Tipo            tipo;           /* int ou car */
    int             linha;          /* linha da declaração */
    int             deslocamento;   /* posição na memória; */
    struct entrada* prox;           /* próxima variável do mesmo escopo */
} Entrada;

/* Um escopo: as variáveis declaradas num bloco. */
typedef struct escopo {
    Entrada*       entradas;        /* variáveis deste escopo */
    struct escopo* abaixo;          /* escopo de fora, mais perto da base */
} Escopo;

/* a) Cria a pilha, inicialmente, vazia. */
void iniciaPilha(void);

/* b) Cria um escopo vazio e o põe no topo da pilha. */
void empilhaEscopo(void);

/* c) Procura "lexema" do topo para a base; devolve a entrada ou NULL. */
Entrada* busca(char* lexema);

/* d) Remove o escopo do topo e libera suas variáveis. */
void desempilhaEscopo(void);

/* Põe a variável no escopo do topo; devolve NULL se o nome já existe nesse escopo. */
Entrada* insere(char* lexema, Tipo tipo, int linha);

/* Mostra os escopos, do topo para a base (para a apresentação). */
void imprimePilha(void);

#endif /* TABELA_H */
