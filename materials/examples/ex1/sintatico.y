%{
/*********** Este arquivo Define a implementação do esquema de traducaoo da figura 2.39, 
************ secao 2.8.2 doDragon book. */
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include "simpleLang.h"
extern char * yytext;
extern int yylex();
extern int numLinha;
extern FILE* yyin;
extern int erroOrigem;
void yyerror( char const *s);

%}
%token IF WHILE DO LEQ  NUM 

%%  /* Secao de regras - producoes da gramatica - Veja as normas de formação de produçoes na 
       secao 3.3 do manual */



program	: 	block               
	;

block	: 	'{' stmts '}'        	
;

stmts 	:	stmt stmts           
        | /*cadeia vazia */          
	;

stmt	:expr ';'		     
        |IF '(' expr ')' stmt        
        |WHILE '(' expr ')' stmt     
        |DO stmt WHILE '(' expr ')'  
        |block              	     
	;

expr	:rel '=' expr         	     
        | rel               	     
	;
rel	: rel  '<' add               
        |rel LEQ add                 
        | add                        
	;
add	: add '+' term               
        | term              	     		
	;
term	: term '*' factor             
        | factor                     
	;
factor	: '(' expr ')'               
	| NUM                        
	;
%% /* Secao Epilogo*/	



int main(int argc, char** argv){
   if(argc!=2)
        yyerror("Uso correto: ./simpleLang nome_arq_entrada");
   yyin=fopen(argv[1], "r");
   if(!yyin)
        yyerror("arquivo não pode ser aberto\n");
    yyparse();    
}

void yyerror( char const *s) {
    
        printf("Erro sintatico proximo a %s ", yytext);
        printf(" - linha: %d \n", numLinha);
        erroOrigem=1;
		exit(1);
}



