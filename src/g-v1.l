%{
/* g-v1.l - analisador léxico: transforma o fonte em tokens */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "g-v1.tab.h"   /* gerado pelo Bison: tokens, yylval e yylloc */

/* Guarda em yylloc a linha de cada token (brancos e miolo de comentário não contam). */
#define YY_USER_ACTION  if (YY_START == INITIAL && !strchr(" \t\r\n", yytext[0])) \
                            yylloc.first_line = yylloc.last_line = yylineno;

void  erroLexico(char* mensagem, int linha);
static int linhaComentario;   /* linha onde o comentário aberto começou */
static char* copia(const char* texto);
%}

%option yylineno noyywrap

DIGITO   [0-9]
LETRA    [A-Za-z_]

/* Estado exclusivo: dentro dele só valem as regras marcadas <COMENTARIO>. */
%x COMENTARIO

%%
 /* Vence o casamento mais longo; no empate, a regra que vem primeiro. */
 /* ---------- comentários ---------- */
"/*"                      { linhaComentario = yylineno; BEGIN(COMENTARIO); }   /* guarda onde abriu */
<COMENTARIO>"*/"          BEGIN(INITIAL);
<COMENTARIO>\n            ;    /* "." não casa \n */
<COMENTARIO>.             ;
<COMENTARIO><<EOF>>       { erroLexico("COMENTÁRIO NAO TERMINA", linhaComentario); }
<<EOF>>                   return EOF;   /* fim do arquivo: devolve -1 */

[ \t\r\n]+                ;    /* brancos: não devolve token */

 /* ---------- palavras reservadas (antes de IDENTIFICADOR) ---------- */
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
"&"                       return E;
"=="                      return IGUAL;
"!="                      return DIFERENTE;
">="                      return MAIORIGUAL;
"<="                      return MENORIGUAL;

[{}():;,=<>+\-*/!]        return yytext[0];    /* o token é o próprio código ASCII */

 /* ---------- identificadores e constantes (levam o lexema) ---------- */
{LETRA}({LETRA}|{DIGITO})*   { yylval.lexema = copia(yytext); return IDENTIFICADOR; }
{DIGITO}+                    { yylval.lexema = copia(yytext); return INTCONST; }           /* sem sinal: o '-' vem da gramática */
'([^'\\\n]|\\.)'             { yylval.lexema = copia(yytext); return CARCONST; }           /* um caractere ou um escape (\n) */
\"[^\"\n]*\"                 { yylval.lexema = copia(yytext); return CADEIACARACTERES; }   /* fecha na primeira aspa */

 /* ---------- erros léxicos ---------- */
\"[^\"\n]*                   { erroLexico("CADEIA DE CARACTERES OCUPA MAIS DE UMA LINHA", yylineno); }   /* aspa sem fechar na linha */
.                            { erroLexico("CARACTERE INVÁLIDO", yylineno); }   /* pega-tudo: sempre por último */

%%
/* O Flex reaproveita yytext a cada token, então o lexema é copiado. */
static char* copia(const char* texto) {
    char* c = strdup(texto);
    if (!c) {
        printf("ERRO: memoria insuficiente\n");
        exit(1);
    }
    return c;
}

/* Mostra o erro com a linha e encerra a compilação. */
void erroLexico(char* mensagem, int linha) {
    printf("ERRO: %s %d\n", mensagem, linha);
    exit(1);
}
