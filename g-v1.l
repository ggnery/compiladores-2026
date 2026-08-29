%{
/* g-v1.l - analisador lexico da linguagem G-V1 (Fase 1)
   Justificativas das decisoes: EXPLICACAO-fase1.md */
#include <stdio.h>
#include <stdlib.h>
#include "g-v1.tab.h"   /* numero de cada token, gerado pelo Bison */

void erroLexico(char* mensagem);
%}

/* conta linhas sozinho, em yylineno */
%option yylineno

/* so ha um arquivo de entrada */
%option noyywrap

DIGITO   [0-9]
LETRA    [A-Za-z_]

/* estado para comentarios, que podem ocupar varias linhas */
%x COMENTARIO

%%
 /* ---------- comentarios ---------- */
"/*"                      BEGIN(COMENTARIO);   /* entra no estado */
<COMENTARIO>"*/"          BEGIN(INITIAL);      /* sai do estado */
<COMENTARIO>\n            ;                    /* descarta (yylineno conta sozinho) */
<COMENTARIO>.             ;                    /* descarta qualquer outro caractere */
<COMENTARIO><<EOF>>       { erroLexico("COMENTÁRIO NAO TERMINA"); }  /* acabou o arquivo dentro do comentario */

 /* ---------- espacos ---------- */
[ \t\n]+                  ;                    /* reconhece e nao devolve token */

 /* ---------- palavras reservadas: antes de IDENTIFICADOR ---------- */
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

 /* ---------- operadores compostos: antes dos simples ---------- */
"||"                      return OU;           /* ou logico */
"&"                       return E;            /* e logico */
"=="                      return IGUAL;
"!="                      return DIFERENTE;
">="                      return MAIORIGUAL;
"<="                      return MENORIGUAL;

 /* ---------- caracteres simples ---------- */
[{}():;,=<>+\-*/!]        return yytext[0];    /* devolve o proprio codigo ASCII */

 /* ---------- identificadores e constantes ---------- */
{LETRA}({LETRA}|{DIGITO})*   return IDENTIFICADOR;      /* letra, depois letras ou digitos */
{DIGITO}+                    return INTCONST;           /* sem sinal: research.md D12 */
'([^'\\\n]|\\.)'             return CARCONST;           /* um caractere, ou um escape */
\"[^\"\n]*\"                 return CADEIACARACTERES;   /* entre aspas, sem quebra de linha */

 /* ---------- erros lexicos ---------- */
\"[^\"\n]*                   { erroLexico("CADEIA DE CARACTERES OCUPA MAIS DE UMA LINHA"); }  /* aspa que nao fecha */
.                            { erroLexico("CARACTERE INVÁLIDO"); }  /* pega-tudo: sempre por ultimo */

%%
/* Formato do enunciado: ERRO: <mensagem> <linha> */
void erroLexico(char* mensagem) {
    printf("ERRO: %s %d\n", mensagem, yylineno);
    exit(1);
}
