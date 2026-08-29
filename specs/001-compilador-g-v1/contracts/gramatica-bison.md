# Contrato 2 — Gramática (entrada do Bison, `g-v1.y`)

Transcrição direta da Seção 2 do enunciado. **Verificado com o Bison: 0 conflitos
shift/reduce e 0 reduce/reduce** — não são necessárias declarações `%left`/`%right`.

## Declarações do arquivo

```
%token PRINCIPAL INT CAR LEIA ESCREVA NOVALINHA SE ENTAO SENAO FIMSE ENQUANTO
%token OU E IGUAL DIFERENTE MAIORIGUAL MENORIGUAL
%token <lexema> IDENTIFICADOR INTCONST CARCONST CADEIACARACTERES
%type  <no> Bloco VarSection ListaDeclVar DeclVar ListaComando Comando
%type  <no> Expr OrExpr AndExpr EqExpr DesigExpr AddExpr MulExpr UnExpr PrimExpr
%start Programa
```

Prólogo obrigatório (Seção 3.2 do enunciado):

```c
extern int   yylineno;
extern char* yytext;
extern int   yylex();
extern FILE* yyin;
void yyerror(char const* s);
```

## Produções

```
Programa      : DeclPrograma ;
DeclPrograma  : PRINCIPAL Bloco ;

Bloco         : '{' ListaComando '}'
              | VarSection '{' ListaComando '}' ;

VarSection    : '{' ListaDeclVar '}' ;

ListaDeclVar  : IDENTIFICADOR DeclVar ':' Tipo ';' ListaDeclVar
              | IDENTIFICADOR DeclVar ':' Tipo ';' ;

DeclVar       : /* vazio */
              | ',' IDENTIFICADOR DeclVar ;

Tipo          : INT | CAR ;

ListaComando  : Comando
              | Comando ListaComando ;

Comando       : ';'
              | Expr ';'
              | LEIA IDENTIFICADOR ';'
              | ESCREVA Expr ';'
              | ESCREVA CADEIACARACTERES ';'
              | NOVALINHA ';'
              | SE '(' Expr ')' ENTAO Comando FIMSE
              | SE '(' Expr ')' ENTAO Comando SENAO Comando FIMSE
              | ENQUANTO '(' Expr ')' Comando
              | Bloco ;

Expr          : OrExpr
              | IDENTIFICADOR '=' Expr ;

OrExpr        : OrExpr OU AndExpr | AndExpr ;
AndExpr       : AndExpr E EqExpr  | EqExpr ;
EqExpr        : EqExpr IGUAL DesigExpr | EqExpr DIFERENTE DesigExpr | DesigExpr ;
DesigExpr     : DesigExpr '<' AddExpr | DesigExpr '>' AddExpr
              | DesigExpr MAIORIGUAL AddExpr | DesigExpr MENORIGUAL AddExpr | AddExpr ;
AddExpr       : AddExpr '+' MulExpr | AddExpr '-' MulExpr | MulExpr ;
MulExpr       : MulExpr '*' UnExpr | MulExpr '/' UnExpr | UnExpr ;
UnExpr        : '-' PrimExpr | '!' PrimExpr | PrimExpr ;
PrimExpr      : IDENTIFICADOR | CARCONST | INTCONST | '(' Expr ')' ;
```

## Notas de leitura da gramática

- **A precedência já está na gramática.** `Expr` desce por níveis até `PrimExpr`; quanto mais
  fundo, mais forte o operador. Por isso `1 + 2 * 3` sai certo sem nenhuma diretiva.
- **Não há bloco vazio**: `ListaComando` exige pelo menos um comando. Um bloco "vazio" se
  escreve `{ ; }`.
- **Um bloco é um comando** (`Comando : Bloco`), e é isso que permite o aninhamento.
- **`senao` não é ambíguo** porque todo `se` fecha com `fimse`.
- Só o lado esquerdo de uma atribuição pode ser um `IDENTIFICADOR` puro — a gramática não
  permite `(a) = 3`.

## Ações semânticas (Fase 2)

Cada produção recebe uma ação `{ $$ = criaNo(...); }` conforme a tabela de filhos em
`data-model.md`. As produções de "repasse" (ex.: `AddExpr : MulExpr`) usam `{ $$ = $1; }`.
