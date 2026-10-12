%{
/* g-v1.l - analisador léxico da linguagem G-V1 (Seção 3.1 e Tabela 1 do enunciado).
   Primeira etapa: fonte -> tokens. O Flex gera daqui build/lexico.c com a função yylex(),
   que o yyparse() do g-v1.y chama sempre que precisa do próximo token.
   Entra: os caracteres de yyin (aberto no main do g-v1.y).
   Sai:   o código do token, o lexema em yylval.lexema e a linha em yylloc;
          num erro léxico, a linha "ERRO: ..." e o fim do programa. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "g-v1.tab.h"   /* gerado pelo Bison: números dos tokens, yylval e yylloc */

/* Roda antes da ação de cada regra: guarda em yylloc a linha do token, que o Bison expõe
   como @1, @2... (a linha gravada nos nós da árvore). Só conta texto lido em INITIAL que não
   começa com espaço (tokens e a abertura de comentário): assim, quando o sintático trava no
   fim do arquivo, o yyerror acusa a linha do último token (ou comentário) e não a do fim. */
#define YY_USER_ACTION  if (YY_START == INITIAL && !strchr(" \t\r\n", yytext[0])) \
                            yylloc.first_line = yylloc.last_line = yylineno;

void  erroLexico(char* mensagem, int linha);
static int linhaComentario;   /* linha onde o comentário aberto começou */
static char* copia(const char* texto);
%}

/* yylineno: o Flex conta as linhas sozinho, inclusive os \n de dentro dos comentários.
   noyywrap: há um só arquivo de entrada, então não é preciso escrever yywrap(). */
%option yylineno noyywrap

/* '_' conta como letra: pela Tabela 1, ele pode iniciar o identificador e aparecer no resto */
DIGITO   [0-9]
LETRA    [A-Za-z_]

/* Estado exclusivo (%x): dentro do comentário só valem as regras marcadas <COMENTARIO>;
   sem isso, uma palavra como "se" escrita no comentário viraria token. */
%x COMENTARIO

%%
 /* Se várias regras casam: 1) vence o casamento mais longo ("<=" e não "<"; "senaox" é IDENTIFICADOR); */
 /* 2) no empate de tamanho, vence a escrita primeiro: por isso as palavras reservadas vêm antes de IDENTIFICADOR. */

 /* ---------- comentários ---------- */
"/*"                      { linhaComentario = yylineno; BEGIN(COMENTARIO); }   /* guarda onde abriu */
<COMENTARIO>"*/"          BEGIN(INITIAL);
<COMENTARIO>\n            ;    /* à parte porque "." não casa \n */
<COMENTARIO>.             ;
<COMENTARIO><<EOF>>       { erroLexico("COMENTÁRIO NAO TERMINA", linhaComentario); }   /* linha da abertura: no EOF o yylineno já passou da última linha */
<<EOF>>                   return EOF;   /* fim do arquivo fora de comentário: -1, como pede a Seção 3.1 (sem a regra o Flex devolveria 0; o Bison aceita qualquer valor <= 0) */

[ \t\r\n]+                ;    /* brancos (\r: fim de linha do Windows): consome sem devolver token */

 /* ---------- palavras reservadas ---------- */
"principal"               return PRINCIPAL;
"int"                     return INT;
"car"                     return CAR;
"leia"                    return LEIA;
"escreva"                 return ESCREVA;
"novalinha"               return NOVALINHA;
"se"                      return SE;
"entao"                   return ENTAO;
"senao"                   return SENAO;
"fimse"                   return FIMSE;
"enquanto"                return ENQUANTO;

 /* ---------- operadores lógicos e relacionais ---------- */
"||"                      return OU;
"&"                       return E;            /* um caractere só, mas a gramática o chama de E */
"=="                      return IGUAL;
"!="                      return DIFERENTE;
">="                      return MAIORIGUAL;
"<="                      return MENORIGUAL;

[{}():;,=<>+\-*/!]        return yytext[0];    /* o token é o próprio código ASCII: na gramática vêm entre apóstrofes, sem %token */

 /* ---------- identificadores e constantes ---------- */
 /* Estes quatro levam o lexema (Seção 3.1): uma cópia do texto vai em yylval.lexema, campo da %union do .y. */
{LETRA}({LETRA}|{DIGITO})*   { yylval.lexema = copia(yytext); return IDENTIFICADOR; }
{DIGITO}+                    { yylval.lexema = copia(yytext); return INTCONST; }           /* sem sinal (D12): com sinal, "a-1" viraria "a" "-1"; o sinal vem da gramática (UnExpr '-', PrimExpr '+') */
'([^'\\\n]|\\.)'             { yylval.lexema = copia(yytext); return CARCONST; }           /* um caractere ou um escape (\n, \t...); o lexema guarda os apóstrofes */
\"[^\"\n]*\"                 { yylval.lexema = copia(yytext); return CADEIACARACTERES; }   /* não há aspa escapada: fecha na primeira aspa; o lexema guarda as aspas */

 /* ---------- erros léxicos ---------- */
 /* Aspa aberta sem fechar na mesma linha. Se a aspa final existe, a regra da cadeia casa 1 caractere a mais e vence. */
\"[^\"\n]*                   { erroLexico("CADEIA DE CARACTERES OCUPA MAIS DE UMA LINHA", yylineno); }
.                            { erroLexico("CARACTERE INVÁLIDO", yylineno); }   /* pega-tudo: sempre por último, só recebe o que nenhuma regra quis (ex.: @) */

%%
/* yytext é um buffer do Flex, sobrescrito a cada token; o lexema vai para a árvore e é usado
   até a geração de código, então precisa de uma cópia própria. */
static char* copia(const char* texto) {
    char* c = strdup(texto);
    if (!c) {
        printf("ERRO: memoria insuficiente\n");
        exit(1);
    }
    return c;
}

/* Formato exigido pela Seção 3.1: "ERRO: <mensagem> <linha>". Para no primeiro erro. */
void erroLexico(char* mensagem, int linha) {
    printf("ERRO: %s %d\n", mensagem, linha);
    exit(1);
}
