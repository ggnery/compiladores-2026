#ifndef AST_H
#define AST_H

/* ast.h - árvore sintática abstrata da linguagem G-V1  */

typedef enum {
    /* estrutura */
    PROGRAMA, BLOCO, LISTA_DECL, DECL, LISTA_CMD,
    /* comandos */
    LEIA_CMD, ESCREVA_CMD, ESCREVA_STR, NOVALINHA_CMD, SE_CMD, ENQUANTO_CMD,
    /* expressões */
    ATRIB, OU_OP, E_OP, IGUAL_OP, DIFERENTE_OP,
    MENOR_OP, MAIOR_OP, MAIORIGUAL_OP, MENORIGUAL_OP,
    MAIS_OP, MENOS_OP, MULT_OP, DIV_OP,
    NEG_OP, NAO_OP,
    /* folhas */
    ID, INT_CONST, CAR_CONST, STR_CONST
} Especie;

typedef enum { TIPO_INT, TIPO_CAR, TIPO_NENHUM } Tipo;

typedef struct no {
    Especie     especie;   /* espécie do nó */
    int         linha;     /* linha no programa fonte */
    char*       lexema;    /* nome ou texto da constante */
    Tipo        tipo;      /* int ou car; TIPO_NENHUM enquanto não se sabe */
    struct no*  filho1;
    struct no*  filho2;
    struct no*  filho3;
} No;

/* Cria um nó da árvore sintática abstrata. */
No* criaNo(Especie especie, int linha, char* lexema, No* f1, No* f2, No* f3);

/* Monta a LISTA_DECL de uma linha "a, b, c : int;" (um DECL por nome, todos com "tipo") e emenda "resto" no fim. */
No* declara(char* nome, int linha, No* outros, Tipo tipo, No* resto);

/* Salva a árvore inteira, em formato de galhos, no arquivo "nomeArquivo". */
void imprimeArvore(No* raiz, const char* nomeArquivo);

/* Retorna uma string da espécie de um nó. */
const char* nomeEspecie(Especie especie);

#endif /* AST_H */
