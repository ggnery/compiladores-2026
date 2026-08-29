
%{
// Secao do Flex de definicao de vriaveis da linguagem C, e inclusap  de arquivos de cabecalho
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include "simpleLang.h"
#include "simpleLang.h"
char msg[500];
extern void yyerror( char const   *s); 
int numLinha=1;
int erroOrigem=1;
%}

/*Secao do Flex destinada a definicoes regulares */

DIGIT [0-9]
%x comentario
%%


{DIGIT}+    return(NUM);
						
							
"/*"		    BEGIN(comentario);
<comentario>[^*\n]*  /*se esta em uma start condition correspondente a comentario, consome 
                                    qualquer caractere que nao seja * ou \n */

<comentario>"*"+[^/\n]  
<comentario><<EOF>> {
                    strcpy(msg, "Erro: comentario nao termina");
		    erroOrigem = 0; /* erro lexico */ 
		    yyerror(msg);
                  }
<comentario>\n numLinha++;    
<comentario>"*"+"/"    BEGIN(INITIAL);  
     
"+"			{return '+';}
"*"			{return '*';}
"{"			{return '{';}
"}"			{return '}';}
"("			{return '(';}

")"			{return ')';}

";"			{return ';';}
"="			{return '=';}
"<"			{return '<';}
"<="		{return LEQ;}

"while"	    {return WHILE;}
"if"		{return IF;}
"do"		{return DO;}

\n 			numLinha++;


[ \t]+ 	; /* consome caracteres de espaco */

<<EOF>> return(EOF);

.			{ 
				erroOrigem=0; /* erro lexico */
				strcpy(msg, "Erro: comentario nao termina");
				yyerror(msg);
			}		

%%

int yywrap(){return 1;}
