%{
/* g-v1.y - analisador sintatico da linguagem G-V1 (Fase 1)
   Gramatica da Secao 2 do enunciado. Detalhes: EXPLICACAO-fase1.md */
#include <stdio.h>
#include <stdlib.h>

/* definidos no codigo gerado pelo Flex, compilado a parte */
extern int   yylineno;
extern char* yytext;
extern int   yylex();
extern FILE* yyin;

void yyerror(char const* s);
%}

/* Tokens de um caractere ('{', ';', ...) nao entram aqui: sao o proprio ASCII. */
%token PRINCIPAL INT CAR LEIA ESCREVA NOVALINHA SE ENTAO SENAO FIMSE ENQUANTO
%token OU E IGUAL DIFERENTE MAIORIGUAL MENORIGUAL
%token IDENTIFICADOR INTCONST CARCONST CADEIACARACTERES

%start Programa

%%
/* ---------- estrutura ---------- */

Programa      : DeclPrograma
              ;

DeclPrograma  : PRINCIPAL Bloco                    /* todo programa comeca com "principal" */
              ;

Bloco         : '{' ListaComando '}'               /* bloco sem declaracoes */
              | VarSection '{' ListaComando '}'    /* bloco com declaracoes: dois pares de chaves */
              ;

VarSection    : '{' ListaDeclVar '}'               /* a secao de variaveis tem chaves proprias */
              ;

/* ---------- declaracoes ---------- */

ListaDeclVar  : IDENTIFICADOR DeclVar ':' Tipo ';' ListaDeclVar   /* ha mais declaracoes */
              | IDENTIFICADOR DeclVar ':' Tipo ';'                /* esta e a ultima */
              ;

DeclVar       : /* vazio - um nome so:  x : int; */
              | ',' IDENTIFICADOR DeclVar          /* varios nomes:  x, y, z : int; */
              ;

Tipo          : INT
              | CAR
              ;

/* ---------- comandos ---------- */

ListaComando  : Comando                            /* o ultimo comando da lista */
              | Comando ListaComando               /* um comando seguido do resto */
              ;

Comando       : ';'                                                 /* comando vazio */
              | Expr ';'                                            /* atribuicao ou expressao */
              | LEIA IDENTIFICADOR ';'                              /* le um valor */
              | ESCREVA Expr ';'                                    /* imprime um valor */
              | ESCREVA CADEIACARACTERES ';'                        /* imprime um texto fixo */
              | NOVALINHA ';'                                       /* pula linha */
              | SE '(' Expr ')' ENTAO Comando FIMSE                 /* se sem senao */
              | SE '(' Expr ')' ENTAO Comando SENAO Comando FIMSE   /* se com senao */
              | ENQUANTO '(' Expr ')' Comando                       /* laco */
              | Bloco                                               /* bloco aninhado */
              ;

/* ---------- expressoes ----------
   A precedencia esta na cascata Expr -> ... -> PrimExpr: quanto mais
   fundo, mais forte o operador. Por isso nao ha %left nem %right. */

Expr          : OrExpr
              | IDENTIFICADOR '=' Expr             /* atribuicao: so um nome a esquerda */
              ;

OrExpr        : OrExpr OU AndExpr                  /* ||  - o operador mais fraco */
              | AndExpr
              ;

AndExpr       : AndExpr E EqExpr                   /* &   */
              | EqExpr
              ;

EqExpr        : EqExpr IGUAL DesigExpr             /* ==  */
              | EqExpr DIFERENTE DesigExpr         /* !=  */
              | DesigExpr
              ;

DesigExpr     : DesigExpr '<' AddExpr              /* <   */
              | DesigExpr '>' AddExpr              /* >   */
              | DesigExpr MAIORIGUAL AddExpr       /* >=  */
              | DesigExpr MENORIGUAL AddExpr       /* <=  */
              | AddExpr
              ;

AddExpr       : AddExpr '+' MulExpr                /* +   */
              | AddExpr '-' MulExpr                /* -   */
              | MulExpr
              ;

MulExpr       : MulExpr '*' UnExpr                 /* *   - mais forte que + e - */
              | MulExpr '/' UnExpr                 /* /   */
              | UnExpr
              ;

UnExpr        : '-' PrimExpr                       /* menos unario:  -5 */
              | '!' PrimExpr                       /* negacao:  !x */
              | PrimExpr
              ;

PrimExpr      : IDENTIFICADOR                      /* nome de variavel */
              | CARCONST                           /* 'a' */
              | INTCONST                           /* 42 */
              | '(' Expr ')'                       /* parenteses mudam a ordem */
              ;

%%

int main(int argc, char** argv) {
    if (argc != 2) {
        printf("Uso correto: ./g-v1 nome_do_arquivo\n");
        return 1;
    }

    yyin = fopen(argv[1], "r");   /* arquivo que o lexico vai ler */
    if (!yyin) {
        printf("Nao foi possivel abrir o arquivo %s\n", argv[1]);
        return 1;
    }

    yyparse();     /* chama yylex() quantas vezes precisar */
    fclose(yyin);

    printf("Programa sintaticamente correto.\n");
    return 0;
}

/* Chamada pelo Bison quando o parser trava. */
void yyerror(char const* s) {
    (void)s;   /* a mensagem do Bison vem em ingles; usamos a nossa */
    printf("ERRO: sintatico proximo a \"%s\" - linha %d\n", yytext, yylineno);
    exit(1);
}
