%{
/* g-v1.l - analisador léxico da linguagem G-V1 */
#include <stdio.h>
#include <stdlib.h>
#include "g-v1.tab.h"   /* número de cada token, gerado pelo Bison */

void erroLexico(char* mensagem);
%}

/* yylineno: o Flex conta as linhas sozinho. noyywrap: só há um arquivo. */
%option yylineno noyywrap

DIGITO   [0-9]
LETRA    [A-Za-z_]

/* Estado exclusivo: dentro dele só valem as regras marcadas <COMENTARIO>. */
%x COMENTARIO

%%
 /* ---------- comentários ---------- */
"/*"                      BEGIN(COMENTARIO);
<COMENTARIO>"*/"          BEGIN(INITIAL);
<COMENTARIO>\n            ;                    /* "." não casa \n; sem esta regra ele seria ecoado */
<COMENTARIO>.             ;
<COMENTARIO><<EOF>>       { erroLexico("COMENTÁRIO NAO TERMINA"); }

[ \t\r\n]+                ;    /* reconhece e nao devolve token */

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
{LETRA}({LETRA}|{DIGITO})*   return IDENTIFICADOR;
{DIGITO}+                    return INTCONST;           /* sem sinal */
'([^'\\\n]|\\.)'             return CARCONST;           /* um caractere, ou um escape */
\"[^\"\n]*\"                 return CADEIACARACTERES;

 /* ---------- erros léxicos ---------- */
\"[^\"\n]*                   { erroLexico("CADEIA DE CARACTERES OCUPA MAIS DE UMA LINHA"); }
.                            { erroLexico("CARACTERE INVÁLIDO"); }   /* pega-tudo: sempre por último */

%%
/* Formato exigido pelo enunciado: ERRO: <mensagem> <linha> */
void erroLexico(char* mensagem) {
    printf("ERRO: %s %d\n", mensagem, yylineno);
    exit(1);
}
