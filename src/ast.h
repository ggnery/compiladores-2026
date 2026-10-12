#ifndef AST_H
#define AST_H

/* ast.h - árvore sintática abstrata (AST) */

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
    ID, INT_CONST, CAR_CONST, STR_CONST   /* STR_CONST não é usada: a cadeia fica no ESCREVA_STR */
} Especie;

typedef enum { TIPO_INT, TIPO_CAR, TIPO_NENHUM } Tipo;

/* Um nó só para tudo; o que cada filho é depende da espécie. Filho ausente é NULL. */
typedef struct no {
    Especie     especie;   /* SE_CMD, MAIS_OP, ID... */
    int         linha;     /* linha no fonte */
    char*       lexema;    /* nome ou texto da constante; NULL nos demais */
    Tipo        tipo;      /* int ou car (DECL e expressões); TIPO_NENHUM nos demais */
    struct no*  filho1;    /* declarações, condição, operando esquerdo, item da lista */
    struct no*  filho2;    /* comandos, então, corpo, operando direito, resto da lista */
    struct no*  filho3;    /* senão */
} No;

/* Cria um nó; chamada nas ações do g-v1.y. */
No* criaNo(Especie especie, int linha, char* lexema, No* f1, No* f2, No* f3);

/* Monta a LISTA_DECL de "a, b, c : int;" (um DECL por nome, com o tipo) e emenda resto no fim. */
No* declara(char* nome, int linha, No* outros, Tipo tipo, No* resto);

/* Grava a árvore, em galhos, no arquivo nomeArquivo. */
void imprimeArvore(No* raiz, const char* nomeArquivo);

/* Nome da espécie como texto ("SE_CMD"). */
const char* nomeEspecie(Especie especie);

#endif /* AST_H */
