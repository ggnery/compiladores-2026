%{
/* g-v1.l - analisador léxico da linguagem G-V1 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>     /* strdup */
#include "g-v1.tab.h"   /* número de cada token, gerado pelo Bison */

/* Roda antes de cada ação: guarda a linha do token, que o parser lê em @1. */
#define YY_USER_ACTION  yylloc.first_line = yylloc.last_line = yylineno;

void  erroLexico(char* mensagem, int linha);
static int linhaComentario;   /* linha onde o comentário aberto começou */
static char* copia(const char* texto);
%}

/* yylineno: o Flex conta as linhas sozinho. noyywrap: só há um arquivo. */
%option yylineno noyywrap

DIGITO   [0-9]
LETRA    [A-Za-z_]

/* Estado exclusivo: dentro dele só valem as regras marcadas <COMENTARIO>. */
%x COMENTARIO

%%
 /* ---------- comentários ---------- */
"/*"                      { linhaComentario = yylineno; BEGIN(COMENTARIO); }   /* guarda onde abriu */
<COMENTARIO>"*/"          BEGIN(INITIAL);
<COMENTARIO>\n            ;                    /* "." não casa \n; sem esta regra ele seria ecoado */
<COMENTARIO>.             ;
<COMENTARIO><<EOF>>       { erroLexico("COMENTÁRIO NAO TERMINA", linhaComentario); }

[ \t\r\n]+                ;    /* espaços: reconhece e não devolve token */

 /* ---------- palavras reservadas: antes da regra de IDENTIFICADOR ---------- */
 /* "principal" casa nas duas com o mesmo tamanho; no empate o Flex usa a primeira. */
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

 /* ---------- operadores de dois símbolos: antes dos de um ---------- */
"||"                      return OU;
"&"                       return E;
"=="                      return IGUAL;
"!="                      return DIFERENTE;
">="                      return MAIORIGUAL;
"<="                      return MENORIGUAL;

[{}():;,=<>+\-*/!]        return yytext[0];    /* o token é o próprio código ASCII */

 /* ---------- identificadores e constantes ---------- */
{LETRA}({LETRA}|{DIGITO})*   { yylval.lexema = copia(yytext); return IDENTIFICADOR; }      /* guarda o nome */
{DIGITO}+                    { yylval.lexema = copia(yytext); return INTCONST; }           /* guarda o número */
'([^'\\\n]|\\.)'             { yylval.lexema = copia(yytext); return CARCONST; }           /* guarda o caractere */
\"[^\"\n]*\"                 { yylval.lexema = copia(yytext); return CADEIACARACTERES; }   /* guarda a cadeia */

 /* ---------- erros léxicos ---------- */
\"[^\"\n]*                   { erroLexico("CADEIA DE CARACTERES OCUPA MAIS DE UMA LINHA", yylineno); }
.                            { erroLexico("CARACTERE INVÁLIDO", yylineno); }   /* pega-tudo: sempre por último */

%%
/* O Flex reaproveita o buffer de yytext a cada token, então o lexema tem de ser
   copiado agora. Aborta igual ao criaNo se faltar memória. */
static char* copia(const char* texto) {
    char* c = strdup(texto);   /* cópia própria do texto */
    if (!c) {
        printf("ERRO: memoria insuficiente\n");
        exit(1);
    }
    return c;
}

/* Formato exigido pelo enunciado: ERRO: <mensagem> <linha> */
void erroLexico(char* mensagem, int linha) {
    printf("ERRO: %s %d\n", mensagem, linha);
    exit(1);
}
