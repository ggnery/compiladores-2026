#ifndef AST_H
#define AST_H

/* ast.h - árvore sintática abstrata (AST) da linguagem G-V1 (Seção 4 do enunciado).
   É a "árvore" de fonte -> tokens -> árvore -> semântico -> MIPS: as ações do g-v1.y a montam
   na memória durante o parse; o semântico (que anota os tipos) e o gerador de MIPS a percorrem,
   sem reler o fonte.
   Compacta, como pede a Seção 4: sem pontuação nem a cascata de precedência da gramática. */

typedef enum {                     /* a ordem tem de bater com nomes[] de nomeEspecie, no ast.c */
    /* estrutura */
    PROGRAMA, BLOCO, LISTA_DECL, DECL, LISTA_CMD,
    /* comandos */
    LEIA_CMD, ESCREVA_CMD, ESCREVA_STR, NOVALINHA_CMD, SE_CMD, ENQUANTO_CMD,
    /* expressões (ATRIB também: "x = e" é uma Expr na gramática) */
    ATRIB, OU_OP, E_OP, IGUAL_OP, DIFERENTE_OP,
    MENOR_OP, MAIOR_OP, MAIORIGUAL_OP, MENORIGUAL_OP,
    MAIS_OP, MENOS_OP, MULT_OP, DIV_OP,
    NEG_OP, NAO_OP,
    /* folhas */
    ID, INT_CONST, CAR_CONST, STR_CONST   /* STR_CONST nunca é criada: a cadeia vai no lexema do ESCREVA_STR */
} Especie;

typedef enum { TIPO_INT, TIPO_CAR, TIPO_NENHUM } Tipo;

/* Uma struct só para todos os nós, para todo percurso ser o mesmo (filho1, filho2, filho3); o
   sentido de cada filho depende da espécie. Ausência (lista vazia, senão ausente, comando ";")
   é NULL, nunca um nó próprio (o "(vazio)" do arvore.txt é só marca da impressão).
     BLOCO: f1 declarações, f2 comandos -- visitando f1 antes de f2, a declaração vem antes do uso (Seção 6)
     LISTA_DECL, LISTA_CMD: f1 um item, f2 o resto (lista encadeada: a struct só tem 3 filhos)
     SE_CMD: f1 condição, f2 então, f3 senão        ENQUANTO_CMD: f1 condição, f2 corpo
     ATRIB e operadores binários: f1 esquerdo, f2 direito        NEG_OP, NAO_OP: só f1
     PROGRAMA: f1 o bloco    LEIA_CMD: f1 o ID    ESCREVA_CMD: f1 a expressão    NOVALINHA_CMD: nada
     DECL, ID, constantes e ESCREVA_STR não têm filhos: o texto fica no lexema */
typedef struct no {
    Especie     especie;   /* o que o nó é: SE_CMD, MAIS_OP, ID... */
    int         linha;     /* linha do fonte (num operador, a do próprio operador), para as mensagens de erro */
    char*       lexema;    /* texto do fonte: nome, 42, 'a', "..." (com as aspas); NULL nos demais */
    Tipo        tipo;      /* DECL: posto no parse (declara); expressões: pelo semântico; demais: TIPO_NENHUM */
    struct no*  filho1;
    struct no*  filho2;
    struct no*  filho3;
} No;

/* Usada nas ações do g-v1.y: quando a regra reduz, os filhos já estão prontos na pilha do
   Bison, então a árvore nasce de baixo para cima. Regras de repasse ($$ = $1) não a chamam. */
No* criaNo(Especie especie, int linha, char* lexema, No* f1, No* f2, No* f3);

/* Monta a LISTA_DECL de uma linha "a, b, c : int;" (um DECL por nome, todos com "tipo") e emenda "resto" no fim. */
No* declara(char* nome, int linha, No* outros, Tipo tipo, No* resto);

/* Salva a árvore inteira, em formato de galhos, no arquivo "nomeArquivo". */
void imprimeArvore(No* raiz, const char* nomeArquivo);

/* Nome da espécie como texto ("SE_CMD"), para gravar a árvore legível. */
const char* nomeEspecie(Especie especie);

#endif /* AST_H */
