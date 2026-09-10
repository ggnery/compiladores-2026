%{
/* g-v1.y - analisador sintático da linguagem G-V1
   Gramática da Seção 2 do enunciado. */
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>   /* stat(): para recusar diretório antes de entregar ao Flex */

/* definidos no código gerado pelo Flex, compilado à parte */
extern int   yylineno;
extern char* yytext;
extern int   yylex();
extern FILE* yyin;

void yyerror(char const* s);
%}

%token PRINCIPAL INT CAR LEIA ESCREVA NOVALINHA SE ENTAO SENAO FIMSE ENQUANTO
%token OU E IGUAL DIFERENTE MAIORIGUAL MENORIGUAL
%token IDENTIFICADOR INTCONST CARCONST CADEIACARACTERES

%start Programa

%%
/* ---------- estrutura ---------- */

Programa      : DeclPrograma
              ;

DeclPrograma  : PRINCIPAL Bloco
              ;

Bloco         : '{' ListaComando '}'
              | VarSection '{' ListaComando '}'    /* com declarações: dois pares de chaves */
              ;

VarSection    : '{' ListaDeclVar '}'
              ;

/* ---------- declarações ---------- */

ListaDeclVar  : IDENTIFICADOR DeclVar ':' Tipo ';' ListaDeclVar
              | IDENTIFICADOR DeclVar ':' Tipo ';'
              ;

DeclVar       : /* vazio */                        /* um nome só:  x : int; */
              | ',' IDENTIFICADOR DeclVar          /* vários nomes:  x, y, z : int; */
              ;

Tipo          : INT
              | CAR
              ;

/* ---------- comandos ---------- */

ListaComando  : Comando
              | Comando ListaComando
              ;

Comando       : ';'                                                 /* comando vazio */
              | Expr ';'
              | LEIA IDENTIFICADOR ';'
              | ESCREVA Expr ';'
              | ESCREVA CADEIACARACTERES ';'
              | NOVALINHA ';'
              | SE '(' Expr ')' ENTAO Comando FIMSE                 /* o FIMSE fecha o se: */
              | SE '(' Expr ')' ENTAO Comando SENAO Comando FIMSE   /* não há senão pendente */
              | ENQUANTO '(' Expr ')' Comando
              | Bloco                                               /* bloco aninhado */
              ;

/* ---------- expressões ----------
   A precedência está na cascata Expr -> OrExpr -> ... -> PrimExpr: quanto mais
   fundo, mais forte o operador. Por isso não há %left nem %right. */

Expr          : OrExpr
              | IDENTIFICADOR '=' Expr             /* atribuição: só um nome à esquerda */
              ;

OrExpr        : OrExpr OU AndExpr
              | AndExpr
              ;

AndExpr       : AndExpr E EqExpr
              | EqExpr
              ;

EqExpr        : EqExpr IGUAL DesigExpr
              | EqExpr DIFERENTE DesigExpr
              | DesigExpr
              ;

DesigExpr     : DesigExpr '<' AddExpr
              | DesigExpr '>' AddExpr
              | DesigExpr MAIORIGUAL AddExpr
              | DesigExpr MENORIGUAL AddExpr
              | AddExpr
              ;

AddExpr       : AddExpr '+' MulExpr
              | AddExpr '-' MulExpr
              | MulExpr
              ;

MulExpr       : MulExpr '*' UnExpr
              | MulExpr '/' UnExpr
              | UnExpr
              ;

UnExpr        : '-' PrimExpr                       /* menos unário:  -5 */
              | '!' PrimExpr
              | PrimExpr
              ;

PrimExpr      : IDENTIFICADOR
              | CARCONST
              | INTCONST
              | '(' Expr ')'
              ;

%%

int main(int argc, char** argv) {
    struct stat info;

    if (argc != 2) {
        printf("Uso correto: ./g-v1 nome_do_arquivo\n");
        return 1;
    }

    /* fopen aceita diretório; quem quebra e' o Flex, com mensagem em inglês. */
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
    return 0;
}

/* Chamada pelo Bison quando o parser trava. */
void yyerror(char const* s) {
    (void)s;   /* a mensagem do Bison vem em inglês; usamos a nossa */
    printf("ERRO: sintatico proximo a \"%s\" - linha %d\n", yytext, yylineno);
    exit(1);
}
