#ifndef AST_H
#define AST_H

/* ast.h - nó da árvore sintática abstrata da linguagem G-V1 (Fase 2)

   Uma struct só para todos os nós: "especie" diz o que o nó é, e os três
   filhos significam coisas diferentes conforme a espécie. A tabela lá
   embaixo é o contrato que dá sentido a filho1/filho2/filho3. */

/* Os sufixos _CMD, _OP e _CONST evitam colisão com os nomes dos tokens que
   o Bison gera em g-v1.tab.h (LEIA, OU, E, IGUAL, ...). */
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
    Especie     especie;
    int         linha;     /* o enunciado exige guardar a linha junto do item léxico */
    char*       lexema;    /* nome ou texto da constante; NULL nas demais espécies */
    Tipo        tipo;      /* só a Fase 4 preenche, exceto em DECL */
    struct no*  filho1;
    struct no*  filho2;
    struct no*  filho3;
} No;

/* Significado dos filhos, por espécie
   ----------------------------------------------------------------------------
   espécie          | filho1              | filho2            | filho3
   -----------------+---------------------+-------------------+-----------------
   PROGRAMA         | bloco principal     | -                 | -
   BLOCO            | lista de declaração | lista de comandos | -   (f1 pode ser NULL)
   LISTA_DECL       | uma DECL            | resto da lista    | -   (f2 NULL no fim)
   LISTA_CMD        | um comando          | resto da lista    | -   (f2 NULL no fim)
   DECL             | -                   | -                 | -   lexema = nome, tipo = int/car
   LEIA_CMD         | nó ID               | -                 | -
   ESCREVA_CMD      | expressão           | -                 | -
   ESCREVA_STR      | -                   | -                 | -   lexema = a cadeia
   NOVALINHA_CMD    | -                   | -                 | -
   SE_CMD           | condição            | comando do então  | comando do senão (NULL se não há)
   ENQUANTO_CMD     | condição            | corpo             | -
   ATRIB            | nó ID (lado esq.)   | expressão (dir.)  | -
   operador binário | operando esquerdo   | operando direito  | -
   NEG_OP, NAO_OP   | operando            | -                 | -
   ID               | -                   | -                 | -   lexema = nome
   *_CONST          | -                   | -                 | -   lexema = o texto lido
   ----------------------------------------------------------------------------
   "operador binário" = de OU_OP a DIV_OP;  "*_CONST" = INT_, CAR_ e STR_CONST.

   Ausência é sempre NULL (lista vazia, senão inexistente), nunca um nó
   "vazio". Assim um único teste de NULL cobre os três casos. */

/* Único ponto do projeto que faz malloc. Aborta se faltar memória.
   "lexema" já deve ser uma cópia: o Flex reaproveita o buffer de yytext. */
No* criaNo(Especie especie, int linha, char* lexema, No* f1, No* f2, No* f3);

/* Imprime a árvore inteira em formato de galhos, a partir da raiz. */
void imprimeArvore(No* raiz);

const char* nomeEspecie(Especie especie);

#endif /* AST_H */
