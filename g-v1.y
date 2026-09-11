%{
/* g-v1.y - analisador sintático da linguagem G-V1
   Gramática da Seção 2 do enunciado. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>     /* strstr(): distinguir estouro de pilha de erro sintático */
#include <sys/stat.h>   /* stat(): recusar diretório antes de entregar ao Flex */

/* definidos no código gerado pelo Flex, compilado à parte */
extern int   yylineno;
extern char* yytext;
extern int   yylex();
extern FILE* yyin;

void yyerror(char const* s);
%}

/* Vai para o g-v1.tab.h, ANTES da %union: o header precisa conhecer No e Tipo
   porque a union os menciona, e o g-v1.l inclui esse header. */
%code requires {
    #include "ast.h"
}

/* Sai no .c depois do include do header, então No e Tipo já existem aqui. */
%code {
    static No* raiz;   /* a árvore pronta; a ação de Programa preenche */
}

/* O valor que cada símbolo carrega na pilha do parser. */
%union {
    char* lexema;   /* tokens que trazem texto */
    No*   no;       /* trecho de árvore já montado */
    Tipo  tipo;     /* só o não-terminal Tipo */
}

%token PRINCIPAL INT CAR LEIA ESCREVA NOVALINHA SE ENTAO SENAO FIMSE ENQUANTO
%token OU E IGUAL DIFERENTE MAIORIGUAL MENORIGUAL

/* estes quatro chegam do Flex com texto junto */
%token <lexema> IDENTIFICADOR INTCONST CARCONST CADEIACARACTERES

%type <no> Programa DeclPrograma Bloco VarSection ListaDeclVar DeclVar
%type <no> ListaComando Comando
%type <no> Expr OrExpr AndExpr EqExpr DesigExpr AddExpr MulExpr UnExpr PrimExpr
%type <tipo> Tipo

/* cada peça da pilha passa a carregar também a linha onde começou (@1, @2, ...) */
%locations

%start Programa

%%
/* ---------- estrutura ---------- */

Programa      : DeclPrograma                                       { raiz = $1; }   /* guarda a árvore pronta */
              ;

DeclPrograma  : PRINCIPAL Bloco                                    { $$ = criaNo(PROGRAMA, @1.first_line, NULL, $2, NULL, NULL); }   /* raiz: o bloco */
              ;

Bloco         : '{' ListaComando '}'                               { $$ = criaNo(BLOCO, @1.first_line, NULL, NULL, $2, NULL); }   /* só comandos */
              | VarSection '{' ListaComando '}'                    { $$ = criaNo(BLOCO, @1.first_line, NULL, $1, $3, NULL); }     /* declarações + comandos */
              ;

VarSection    : '{' ListaDeclVar '}'                               { $$ = $2; }   /* repassa a lista, sem as chaves */
              ;

/* ---------- declarações ----------
   O tipo só aparece depois dos nomes, então declara() (ast.c) volta na lista já
   montada e carimba o tipo em cada DECL daquela linha. */

ListaDeclVar  : IDENTIFICADOR DeclVar ':' Tipo ';' ListaDeclVar    { $$ = declara($1, @1.first_line, $2, $4, $6); }     /* esta linha + as seguintes */
              | IDENTIFICADOR DeclVar ':' Tipo ';'                 { $$ = declara($1, @1.first_line, $2, $4, NULL); }   /* última linha */
              ;

DeclVar       : /* vazio */                                        { $$ = NULL; }                                                                                                   /* só um nome:  x : int; */
              | ',' IDENTIFICADOR DeclVar                          { $$ = criaNo(LISTA_DECL, @2.first_line, NULL, criaNo(DECL, @2.first_line, $2, NULL, NULL, NULL), $3, NULL); }   /* mais um nome:  , y */
              ;

Tipo          : INT                                                { $$ = TIPO_INT; }   /* int */
              | CAR                                                { $$ = TIPO_CAR; }   /* car */
              ;

/* ---------- comandos ----------
   O comando vazio devolve NULL e a lista o descarta: nó "vazio" não existe.
   O FIMSE fecha o se, por isso não há senão pendente. */

ListaComando  : Comando                                            { $$ = $1 ? criaNo(LISTA_CMD, $1->linha, NULL, $1, NULL, NULL) : NULL; }   /* último comando */
              | Comando ListaComando                               { $$ = $1 ? criaNo(LISTA_CMD, $1->linha, NULL, $1, $2, NULL) : $2; }       /* comando + resto */
              ;

Comando       : ';'                                                { $$ = NULL; }                                                                                                 /* vazio: não gera nó */
              | Expr ';'                                           { $$ = $1; }                                                                                                   /* expressão solta */
              | LEIA IDENTIFICADOR ';'                             { $$ = criaNo(LEIA_CMD, @1.first_line, NULL, criaNo(ID, @2.first_line, $2, NULL, NULL, NULL), NULL, NULL); }   /* leia: a variável */
              | ESCREVA Expr ';'                                   { $$ = criaNo(ESCREVA_CMD, @1.first_line, NULL, $2, NULL, NULL); }                                             /* escreva: a expressão */
              | ESCREVA CADEIACARACTERES ';'                       { $$ = criaNo(ESCREVA_STR, @1.first_line, $2, NULL, NULL, NULL); }                                             /* escreva: o texto */
              | NOVALINHA ';'                                      { $$ = criaNo(NOVALINHA_CMD, @1.first_line, NULL, NULL, NULL, NULL); }                                         /* novalinha: sem filhos */
              | SE '(' Expr ')' ENTAO Comando FIMSE                { $$ = criaNo(SE_CMD, @1.first_line, NULL, $3, $6, NULL); }                                                    /* se: condição, então */
              | SE '(' Expr ')' ENTAO Comando SENAO Comando FIMSE  { $$ = criaNo(SE_CMD, @1.first_line, NULL, $3, $6, $8); }                                                      /* se: condição, então, senão */
              | ENQUANTO '(' Expr ')' Comando                      { $$ = criaNo(ENQUANTO_CMD, @1.first_line, NULL, $3, $5, NULL); }                                              /* enquanto: condição, corpo */
              | Bloco                                              { $$ = $1; }                                                                                                   /* bloco aninhado */
              ;

/* ---------- expressões ----------
   A precedência está na cascata Expr -> OrExpr -> ... -> PrimExpr: quanto mais
   fundo, mais forte o operador. Por isso não há %left nem %right.
   Cada regra de operador vira um nó; cada regra de passagem só repassa $1. */

Expr          : OrExpr                                             { $$ = $1; }                                                                                              /* repassa */
              | IDENTIFICADOR '=' Expr                             { $$ = criaNo(ATRIB, @2.first_line, NULL, criaNo(ID, @1.first_line, $1, NULL, NULL, NULL), $3, NULL); }   /* variável = expressão */
              ;

OrExpr        : OrExpr OU AndExpr                                  { $$ = criaNo(OU_OP, @2.first_line, NULL, $1, $3, NULL); }   /* a || b */
              | AndExpr                                            { $$ = $1; }                                                 /* repassa */
              ;

AndExpr       : AndExpr E EqExpr                                   { $$ = criaNo(E_OP, @2.first_line, NULL, $1, $3, NULL); }   /* a & b */
              | EqExpr                                             { $$ = $1; }                                                /* repassa */
              ;

EqExpr        : EqExpr IGUAL DesigExpr                             { $$ = criaNo(IGUAL_OP, @2.first_line, NULL, $1, $3, NULL); }       /* a == b */
              | EqExpr DIFERENTE DesigExpr                         { $$ = criaNo(DIFERENTE_OP, @2.first_line, NULL, $1, $3, NULL); }   /* a != b */
              | DesigExpr                                          { $$ = $1; }                                                        /* repassa */
              ;

DesigExpr     : DesigExpr '<' AddExpr                              { $$ = criaNo(MENOR_OP, @2.first_line, NULL, $1, $3, NULL); }        /* a < b */
              | DesigExpr '>' AddExpr                              { $$ = criaNo(MAIOR_OP, @2.first_line, NULL, $1, $3, NULL); }        /* a > b */
              | DesigExpr MAIORIGUAL AddExpr                       { $$ = criaNo(MAIORIGUAL_OP, @2.first_line, NULL, $1, $3, NULL); }   /* a >= b */
              | DesigExpr MENORIGUAL AddExpr                       { $$ = criaNo(MENORIGUAL_OP, @2.first_line, NULL, $1, $3, NULL); }   /* a <= b */
              | AddExpr                                            { $$ = $1; }                                                         /* repassa */
              ;

AddExpr       : AddExpr '+' MulExpr                                { $$ = criaNo(MAIS_OP, @2.first_line, NULL, $1, $3, NULL); }    /* a + b */
              | AddExpr '-' MulExpr                                { $$ = criaNo(MENOS_OP, @2.first_line, NULL, $1, $3, NULL); }   /* a - b */
              | MulExpr                                            { $$ = $1; }                                                    /* repassa */
              ;

MulExpr       : MulExpr '*' UnExpr                                 { $$ = criaNo(MULT_OP, @2.first_line, NULL, $1, $3, NULL); }   /* a * b */
              | MulExpr '/' UnExpr                                 { $$ = criaNo(DIV_OP, @2.first_line, NULL, $1, $3, NULL); }    /* a / b */
              | UnExpr                                             { $$ = $1; }                                                   /* repassa */
              ;

UnExpr        : '-' PrimExpr                                       { $$ = criaNo(NEG_OP, @1.first_line, NULL, $2, NULL, NULL); }   /* -a */
              | '!' PrimExpr                                       { $$ = criaNo(NAO_OP, @1.first_line, NULL, $2, NULL, NULL); }   /* !a */
              | PrimExpr                                           { $$ = $1; }                                                    /* repassa */
              ;

PrimExpr      : IDENTIFICADOR                                      { $$ = criaNo(ID, @1.first_line, $1, NULL, NULL, NULL); }          /* variável */
              | CARCONST                                           { $$ = criaNo(CAR_CONST, @1.first_line, $1, NULL, NULL, NULL); }   /* 'a' */
              | INTCONST                                           { $$ = criaNo(INT_CONST, @1.first_line, $1, NULL, NULL, NULL); }   /* 42 */
              | '(' Expr ')'                                       { $$ = $2; }                                                       /* repassa, sem os parênteses */
              ;

%%

int main(int argc, char** argv) {
    struct stat info;
    const char* arquivoArvore = "build/arvore.txt";   /* onde a árvore é gravada */

    if (argc != 2) {
        printf("Uso correto: ./g-v1 nome_do_arquivo\n");
        return 1;
    }

    /* fopen aceita diretório; quem quebra é o Flex, com mensagem em inglês. */
    if (stat(argv[1], &info) == 0 && S_ISDIR(info.st_mode)) {
        printf("%s é um diretório, não um arquivo\n", argv[1]);
        return 1;
    }

    yyin = fopen(argv[1], "r");   /* arquivo que o léxico vai ler */
    if (!yyin) {
        printf("Não foi possível abrir o arquivo %s\n", argv[1]);
        return 1;
    }

    yyparse();     /* chama yylex() quantas vezes precisar */
    fclose(yyin);

    printf("Programa sintaticamente correto.\n");
    mkdir("build", 0777);                       /* garante a pasta; se já existe, não faz nada */
    imprimeArvore(raiz, arquivoArvore);         /* grava a árvore no arquivo */
    printf("Árvore gravada em %s\n", arquivoArvore);
    return 0;
}

/* Chamada pelo Bison quando o parser trava. */
void yyerror(char const* s) {
    /* O Bison chama yyerror também quando a pilha estoura; ali não há token
       culpado, e chamar de erro sintático culparia um programa correto. */
    if (s && strstr(s, "memory")) {
        printf("ERRO: programa complexo demais para o analisador - linha %d\n", yylineno);
        exit(1);
    }
    printf("ERRO: sintatico proximo a \"%s\" - linha %d\n", yytext, yylineno);
    exit(1);
}
