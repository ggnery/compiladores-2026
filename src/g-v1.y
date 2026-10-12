%{
/* g-v1.y - analisador sintático da linguagem G-V1 (entrada do Bison)
   Segunda etapa de fonte -> tokens -> árvore -> semântico -> MIPS. O Bison gera daqui
   build/sintatico.c com o yyparse(), que pede tokens ao yylex() (g-v1.l) e, nas reduções,
   monta a árvore sintática abstrata (sai em raiz) ou para com "ERRO: sintatico ...".
   Seção 2: a gramática; Seção 3.2: externs, yyerror e main; Seção 4: as ações que montam a
   árvore. O main, no fim do arquivo, abre o fonte e chama as fases seguintes. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "semantico.h"
#include "codigo.h"

/* Seção 3.2: definidos no lexico.c, gerado pelo Flex. O extern avisa ao compilador e ao
   ligador que eles existem em outro módulo. */
extern int   yylineno;
extern char* yytext;
extern int   yylex();
extern FILE* yyin;

/* O código gerado chama yyerror antes da definição, que fica no fim deste arquivo. */
void yyerror(char const* s);
%}

/* Vai também para o g-v1.tab.h: a %union usa No e Tipo, e o léxico inclui esse header. */
%code requires {
    #include "ast.h"
}

/* Raiz da árvore sintática abstrata: o yyparse() só devolve um int, então a árvore sai por aqui. */
%code {
    static No* raiz;   /* a árvore pronta; a ação de Programa preenche */
}

/* O valor que cada símbolo carrega na pilha do parser. */
%union {
    char* lexema;   /* tokens com texto (o léxico faz yylval.lexema = copia(yytext)) */
    No*   no;       /* não-terminais: a subárvore já montada */
    Tipo  tipo;     /* só o não-terminal Tipo */
}

/* Tokens escritos em maiúsculas na gramática (Tabela 1; até o E, que é só '&'). O Bison os
   numera a partir de 258, acima dos códigos ASCII, no g-v1.tab.h, que o léxico inclui. Os
   que a gramática escreve entre apóstrofes ('{', ';', '+'...) dispensam %token: o léxico
   devolve o próprio código ASCII. */
%token PRINCIPAL INT CAR LEIA ESCREVA NOVALINHA SE ENTAO SENAO FIMSE ENQUANTO
%token OU E IGUAL DIFERENTE MAIORIGUAL MENORIGUAL

/* Os quatro tokens cujo lexema vai para a árvore: nome, número, caractere e cadeia. */
%token <lexema> IDENTIFICADOR INTCONST CARCONST CADEIACARACTERES

/* Qual membro da %union cada não-terminal usa; assim o Bison sabe o tipo C de cada $n. */
%type <no> Programa DeclPrograma Bloco VarSection ListaDeclVar DeclVar
%type <no> ListaComando Comando
%type <no> Expr OrExpr AndExpr EqExpr DesigExpr AddExpr MulExpr UnExpr PrimExpr
%type <tipo> Tipo

/* Liga o @n: cada posição da pilha guarda também a linha do símbolo, tirada do yylloc que o
   YY_USER_ACTION do g-v1.l preenche a cada token (num não-terminal, a do seu primeiro token). */
%locations

%start Programa

%%
/* Gramática da Seção 2, regra por regra; a única produção a mais é PrimExpr -> '+' INTCONST.
   O parser é LALR(1), de baixo para cima: quando uma regra reduz, as subárvores dos filhos já
   estão prontas na pilha ($1, $2...), e a ação as pendura num nó novo com
   criaNo(espécie, linha, lexema, filho1, filho2, filho3). @n.first_line é a linha do
   n-ésimo símbolo da regra, contando a pontuação. */

/* ---------- estrutura ---------- */

Programa      : DeclPrograma                                       { raiz = $1; }   /* última redução: guarda a árvore pronta */
              ;

DeclPrograma  : PRINCIPAL Bloco                                    { $$ = criaNo(PROGRAMA, @1.first_line, NULL, $2, NULL, NULL); }   /* raiz da árvore; filho1 = bloco principal */
              ;

/* BLOCO: declarações sempre em filho1 (NULL sem VarSection) e comandos em filho2. O semântico
   visita filho1 antes de filho2, então os nomes já estão na tabela quando o uso aparece. */
Bloco         : '{' ListaComando '}'                               { $$ = criaNo(BLOCO, @1.first_line, NULL, NULL, $2, NULL); }   /* só comandos */
              | VarSection '{' ListaComando '}'                    { $$ = criaNo(BLOCO, @1.first_line, NULL, $1, $3, NULL); }     /* declarações + comandos */
              ;

VarSection    : '{' ListaDeclVar '}'                               { $$ = $2; }   /* repassa a lista, sem as chaves */
              ;

/* ---------- declarações ---------- */

/* Uma linha "x, y : int;" por vez. O tipo só aparece depois dos nomes, então declara() (ast.c)
   carimba o tipo em cada DECL desta linha e só depois emenda as linhas seguintes ($6). */
ListaDeclVar  : IDENTIFICADOR DeclVar ':' Tipo ';' ListaDeclVar    { $$ = declara($1, @1.first_line, $2, $4, $6); }     /* esta linha + as seguintes */
              | IDENTIFICADOR DeclVar ':' Tipo ';'                 { $$ = declara($1, @1.first_line, $2, $4, NULL); }   /* última linha */
              ;

/* Os outros nomes da linha, numa LISTA_DECL encadeada por filho2; os DECL ainda sem tipo. */
DeclVar       : /* vazio */                                        { $$ = NULL; }                                                                                                   /* só um nome:  x : int; */
              | ',' IDENTIFICADOR DeclVar                          { $$ = criaNo(LISTA_DECL, @2.first_line, NULL, criaNo(DECL, @2.first_line, $2, NULL, NULL, NULL), $3, NULL); }   /* mais um nome:  , y */
              ;

Tipo          : INT                                                { $$ = TIPO_INT; }   /* int */
              | CAR                                                { $$ = TIPO_CAR; }   /* car */
              ;

/* ---------- comandos ---------- */

/* Lista encadeada por filho2 (o nó só tem três filhos): um comando em filho1, o resto em
   filho2, com a linha do comando. O comando vazio ';' chega como NULL e fica fora da lista. */
ListaComando  : Comando                                            { $$ = $1 ? criaNo(LISTA_CMD, $1->linha, NULL, $1, NULL, NULL) : NULL; }   /* último comando */
              | Comando ListaComando                               { $$ = $1 ? criaNo(LISTA_CMD, $1->linha, NULL, $1, $2, NULL) : $2; }       /* comando + resto */
              ;

/* Como Comando pode ser NULL (';'), o então, o senão e o corpo do enquanto também podem ser.
   O fimse obrigatório tira a ambiguidade do senao (dangling else). */
Comando       : ';'                                                { $$ = NULL; }                                                                                                 /* vazio: não gera nó */
              | Expr ';'                                           { $$ = $1; }                                                                                                   /* expressão solta */
              | LEIA IDENTIFICADOR ';'                             { $$ = criaNo(LEIA_CMD, @1.first_line, NULL, criaNo(ID, @2.first_line, $2, NULL, NULL, NULL), NULL, NULL); }   /* leia: a variável */
              | ESCREVA Expr ';'                                   { $$ = criaNo(ESCREVA_CMD, @1.first_line, NULL, $2, NULL, NULL); }                                             /* escreva: a expressão */
              | ESCREVA CADEIACARACTERES ';'                       { $$ = criaNo(ESCREVA_STR, @1.first_line, $2, NULL, NULL, NULL); }                                             /* escreva: a cadeia vai no lexema do nó */
              | NOVALINHA ';'                                      { $$ = criaNo(NOVALINHA_CMD, @1.first_line, NULL, NULL, NULL, NULL); }                                         /* novalinha: sem filhos */
              | SE '(' Expr ')' ENTAO Comando FIMSE                { $$ = criaNo(SE_CMD, @1.first_line, NULL, $3, $6, NULL); }                                                    /* se: condição, então */
              | SE '(' Expr ')' ENTAO Comando SENAO Comando FIMSE  { $$ = criaNo(SE_CMD, @1.first_line, NULL, $3, $6, $8); }                                                      /* se: condição, então, senão */
              | ENQUANTO '(' Expr ')' Comando                      { $$ = criaNo(ENQUANTO_CMD, @1.first_line, NULL, $3, $5, NULL); }                                              /* enquanto: condição, corpo */
              | Bloco                                              { $$ = $1; }                                                                                                   /* bloco aninhado */
              ;

/* ---------- expressões ---------- */

/* Precedência pela cascata da Seção 2: cada nível só desce para o seguinte, então o mais fundo
   amarra mais forte: 1+2*3 = 1+(2*3). A recursão à esquerda dá associatividade à esquerda:
   10-3-2 = (10-3)-2. Por isso não há %left/%right, e o Bison não acusa conflito.
   Operador binário: filho1 e filho2 são os operandos, e a linha é a do operador (@2), usada
   nos erros de tipo do semântico. "repassa" ($$ = $1) não cria nó: a árvore fica compacta. */

/* Atribuição é expressão e, recursiva à direita, associa à direita: x = y = 1 vira
   ATRIB(x, ATRIB(y, 1)). Depois de um IDENTIFICADOR, o lookahead '=' decide entre
   atribuição e PrimExpr. */
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

/* Unário só sobre PrimExpr, como na Seção 2: -(-x) vale, --x não. O -5 vira NEG_OP sobre
   INT_CONST 5, porque o léxico não lê sinal (D12). */
UnExpr        : '-' PrimExpr                                       { $$ = criaNo(NEG_OP, @1.first_line, NULL, $2, NULL, NULL); }   /* -a */
              | '!' PrimExpr                                       { $$ = criaNo(NAO_OP, @1.first_line, NULL, $2, NULL, NULL); }   /* !a */
              | PrimExpr                                           { $$ = $1; }                                                    /* repassa */
              ;

/* '+' INTCONST não está na Seção 2: entra porque a Tabela 1 permite INTCONST com sinal. O '-'
   já vem de UnExpr; o sinal não fica no léxico (D12) porque "a-1" viraria "a" "-1".
   O '+' não muda o valor e não gera nó; +x continua sendo erro. */
PrimExpr      : IDENTIFICADOR                                      { $$ = criaNo(ID, @1.first_line, $1, NULL, NULL, NULL); }          /* variável */
              | CARCONST                                           { $$ = criaNo(CAR_CONST, @1.first_line, $1, NULL, NULL, NULL); }   /* 'a' */
              | INTCONST                                           { $$ = criaNo(INT_CONST, @1.first_line, $1, NULL, NULL, NULL); }   /* 42 */
              | '+' INTCONST                                       { $$ = criaNo(INT_CONST, @2.first_line, $2, NULL, NULL, NULL); }   /* +42: sinal da Tabela 1 */
              | '(' Expr ')'                                       { $$ = $2; }                                                       /* repassa, sem os parênteses */
              ;

%%

/* Seção 3.2: ./g-v1 teste.g. O arquivo vem em argv[1] e é aberto em yyin, de onde o yylex() lê.
   Encadeia as fases; a que acha erro imprime "ERRO: ..." e encerra com exit(1), então cada
   fase só roda se as anteriores passaram. */
int main(int argc, char** argv) {
    struct stat info;
    const char* arquivoArvore = "build/arvore.txt";   /* onde a árvore é gravada */
    char        arquivoAsm[1024];                     /* teste.g -> teste.asm */
    char*       ponto;

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

    yyparse();     /* chama yylex() quantas vezes precisar; erro léxico ou sintático encerra lá dentro */
    fclose(yyin);  /* Seção 3.2: fechar o arquivo ao fim da análise sintática */
    printf("Programa sintaticamente correto.\n");

    analisaSemantica(raiz);                     /* no primeiro erro, avisa e encerra */
    printf("Programa semanticamente correto.\n");

    mkdir("build", 0777);                       /* garante a pasta; se já existe, não faz nada */
    imprimeArvore(raiz, arquivoArvore);         /* grava a árvore, já com os tipos anotados */
    printf("Árvore gravada em %s\n", arquivoArvore);

    snprintf(arquivoAsm, sizeof arquivoAsm - 4, "%s", argv[1]);   /* -4: espaço para o ".asm" */
    ponto = strrchr(arquivoAsm, '.');
    if (ponto && !strchr(ponto, '/')) *ponto = '\0';  /* tira a extensão; um '.' com '/' depois é de diretório (../x) */
    strcat(arquivoAsm, ".asm");
    geraCodigo(raiz, arquivoAsm);               /* só chega aqui se o semântico passou */
    printf("Código assembly gravado em %s\n", arquivoAsm);
    return 0;
}

/* Seção 3.2: o yyparse() chama yyerror quando nenhuma regra aceita o próximo token. A mensagem
   começa com "ERRO: " e traz a linha; o compilador para no primeiro erro. O texto s do Bison
   ("syntax error") é trocado pelo token em que o parser travou. */
void yyerror(char const* s) {
    /* "memory exhausted": a pilha do parser passou de 10000 posições (YYMAXDEPTH), p.ex. com
       ~10000 parênteses aninhados ou comandos num bloco (ListaComando é recursiva à direita). */
    if (s && strstr(s, "memory")) {
        printf("ERRO: programa complexo demais para o analisador - linha %d\n", yylineno);
        exit(1);
    }
    /* No fim do arquivo, yylineno já contou o '\n' final e as linhas em branco (pode apontar uma
       linha que nem existe); yylloc guarda a linha do último token ou abertura de comentário
       lido, gravada pelo YY_USER_ACTION do g-v1.l. */
    if (yytext[0] == '\0') {                   /* travou no fim do arquivo: não há token */
        printf("ERRO: sintatico proximo ao fim do arquivo - linha %d\n", yylloc.first_line);
        exit(1);
    }
    /* yytext e yylineno são do token em que o parser travou (o lookahead). Um ';' faltando no
       fim da linha 20 só é notado no "escreva" da linha 21, e é essa linha que sai: onde o erro
       foi detectado, como pede a Seção 3.2 (teste NotaEmConceitoPonto-e-vrigulaFaltandoLin20). */
    printf("ERRO: sintatico proximo a \"%s\" - linha %d\n", yytext, yylineno);
    exit(1);
}
