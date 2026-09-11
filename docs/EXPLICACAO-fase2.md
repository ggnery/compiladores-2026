# Fase 2 — Árvore sintática abstrata

Roteiro para explicar esta entrega. Arquivos: `ast.h`, `ast.c`, e as ações em `g-v1.y` e `g-v1.l`.

## 1. O problema

Na Fase 1 o compilador só respondia **"o programa é válido?"**. As próximas fases — análise
semântica e geração de código — precisam **percorrer o programa várias vezes**. A solução é
montar, durante a análise sintática, uma **árvore na memória** que representa o programa.

Ela é **abstrata** porque guarda só o que importa para o significado. Para `x = 2 + 3 * 4;`:

```
ATRIB
├── ID x
└── MAIS_OP
    ├── INT_CONST 2
    └── MULT_OP
        ├── INT_CONST 3
        └── INT_CONST 4
```

Não há `;`, nem parênteses, nem a cascata `Expr → OrExpr → … → PrimExpr` que a gramática
atravessa para chegar a cada número. A precedência não some: ela aparece na **forma** da
árvore — o `*` ficou mais fundo que o `+`. É o que o enunciado pede: a árvore "a mais compacta
possível".

## 2. O nó (`ast.h`)

Uma `struct` só para todos os nós:

| campo | guarda |
|---|---|
| `especie` | o que o nó é: `SE_CMD`, `MAIS_OP`, `ID`... |
| `linha` | a linha do fonte — o enunciado exige, para as mensagens de erro |
| `lexema` | o texto do fonte (`soma`, `42`, `"fim"`); `NULL` nos nós sem texto |
| `tipo` | `int` ou `car`; nesta fase só o `DECL` recebe |
| `filho1`, `filho2`, `filho3` | os filhos |

O significado de cada filho **depende da espécie** e está na tabela do `ast.h`. Exemplos:

| espécie | filho1 | filho2 | filho3 |
|---|---|---|---|
| `SE_CMD` | condição | comando do `entao` (ou `NULL`) | comando do `senao` (ou `NULL`) |
| `BLOCO` | declarações (ou `NULL`) | comandos (ou `NULL`) | — |
| `LISTA_CMD` | um comando | resto da lista | — |

Duas regras valem para a árvore inteira:

1. **Em `BLOCO`, declarações à esquerda e comandos à direita.** Percorrendo `filho1` antes de
   `filho2`, todo nome já foi declarado quando o uso aparece — é o que a Fase 4 vai usar.
2. **Ausência é sempre `NULL`**, nunca um nó "vazio": lista vazia, `senao` inexistente e
   comando vazio (`;`). Por isso o `entao`, o `senao` e o corpo do `enquanto` também podem ser
   `NULL` — e, na árvore gravada, um filho `NULL` simplesmente não aparece.

## 3. Por que a pilha do Bison carrega ponteiros

O `yyparse()` trabalha com uma **pilha**: empilha tokens (*shift*) e troca os símbolos do topo
por um só (*reduce*). Quem escolhe entre os dois é o autômato do Bison, olhando o estado atual
e, quando precisa, o próximo token (*lookahead*). Casar com uma regra não basta: com
`AddExpr '+' MulExpr` no topo e `*` chegando, ele empilha o `*` em vez de reduzir — é por isso
que o `*` fica mais fundo na árvore. Para `escreva 1;`:

```
pilha: ESCREVA INTCONST        reduz PrimExpr : INTCONST
pilha: ESCREVA PrimExpr        ... sobe a cascata até Expr
pilha: ESCREVA Expr ';'        reduz Comando : ESCREVA Expr ';'
pilha: Comando
```

Cada posição da pilha carrega também um **valor**. A `%union` diz quais valores existem:

```c
%union {
    char* lexema;   /* tokens que trazem texto */
    No*   no;       /* trecho de árvore já montado */
    Tipo  tipo;     /* só o não-terminal Tipo */
}
```

A ideia central: quando uma regra reduz, **os filhos dela já estão prontos na pilha**, como
ponteiros para subárvores. A ação só cria o nó novo, pendura os filhos nele e deixa na pilha
o ponteiro do resultado. A árvore nasce **de baixo para cima**, na ordem das reduções.

`%token <lexema>` e `%type <no>` dizem qual membro da `%union` cada símbolo usa — assim o
Bison sabe sozinho que o `$2` de `ESCREVA Expr ';'` é um `No*`.

## 4. O que `$$`, `$1`, `$3` e `@1` significam

Cada símbolo do lado direito da regra é numerado, **contando a pontuação**:

```
Bloco : VarSection  '{'  ListaComando  '}'
            $1       $2       $3        $4
```

- **`$n`** — o valor do símbolo *n* (aqui, um `No*` já montado);
- **`$$`** — o valor que a regra produz, isto é, o do símbolo da esquerda;
- **`@n.first_line`** — a linha onde o símbolo *n* começou.

```bison
{ $$ = criaNo(BLOCO, @1.first_line, NULL, $1, $3, NULL); }
```

Lê-se: "o `Bloco` é um nó `BLOCO`, na linha do primeiro símbolo, com as declarações (`$1`) em
`filho1` e os comandos (`$3`) em `filho2`". Na outra forma, `'{' ListaComando '}'`, não há
declarações e os comandos são o `$2`.

## 5. De onde vêm o lexema e a linha (`g-v1.l`)

**Lexema.** Os quatro tokens com texto fazem `yylval.lexema = copia(yytext);`. A cópia é
obrigatória: o Flex **reaproveita** o buffer de `yytext` no token seguinte.

**Linha.** Por que não usar `yylineno` direto na ação? Porque quando a ação roda, o parser
**pode já ter lido o próximo token** (o *lookahead*), e aí `yylineno` já avançou. A solução:

```c
#define YY_USER_ACTION  yylloc.first_line = yylloc.last_line = yylineno;
```

`YY_USER_ACTION` roda em **todo** token, no momento em que ele é lido, e grava a linha em
`yylloc`. Com `%locations` no `g-v1.y`, cada posição da pilha guarda essa linha — é o `@n`.
Com uma expressão quebrada em três linhas:

```
5:  escreva
6:     x
7:     + 2;
```

o `ESCREVA_CMD` fica com a linha 5, o `ID x` com a 6 e o `MAIS_OP` com a 7.

## 6. Três decisões de construção

**Regras de repasse não criam nó.** `Expr : OrExpr { $$ = $1; }` só passa o ponteiro adiante.
É isso que deixa a árvore compacta: a cascata de precedência não vira nó.

**Comando vazio não gera nó.** `Comando : ';'` devolve `NULL`, e a lista o descarta:
`principal { ; ; escreva 1; }` gera uma lista com **um** comando.

**`x, y, z : int;` vira três `DECL`.** O tipo só aparece **depois** dos nomes, então a função
`declara()`, do `ast.c`, monta a lista da linha, carimba o tipo em cada `DECL` e só então emenda as
declarações das linhas seguintes — na ordem inversa, o tipo vazaria para elas.

## 7. Como demonstrar

```bash
make
./g-v1 testes/Outros/ok1.g     # Programa sintaticamente correto.
                               # Árvore gravada em build/arvore.txt
cat build/arvore.txt
```

```
PROGRAMA  (linha 5)
└── BLOCO  (linha 6)
    ├── LISTA_DECL  (linha 6)
    │   ├── DECL soma : int  (linha 6)
    │   └── LISTA_DECL  (linha 6)
    │       ├── DECL i : int  (linha 6)
    │       └── LISTA_DECL  (linha 7)
    │           └── DECL c : car  (linha 7)
    └── LISTA_CMD  (linha 10)
        ...
```

O que conferir: declarações à esquerda do `BLOCO`, comandos à direita, e o `SE_CMD` da linha
23 com **três** filhos — a condição e os dois `escreva`.

A gravação fica em `ast.c`: `imprimeArvore` abre o arquivo, chama `imprimeNo` na raiz e fecha
o arquivo. `imprimeNo` é a função recursiva — cada nó grava o prefixo herdado do pai, o seu
galho e o rótulo, e chama `imprimeNo` para cada filho que não é `NULL`. O último filho usa
`└──`; os outros, `├──`. O nome do arquivo (`build/arvore.txt`) é definido no `main` do `g-v1.y`.

## 8. Perguntas que podem cair

**Por que uma `struct` só, e não uma por construção?** O percurso fica igual para todo nó —
visitar `filho1`, `filho2`, `filho3` —, então uma única função recursiva (`imprimeNo`) grava a árvore
inteira. O preço: o C não sabe o que `filho2` significa. Quem garante é a tabela do `ast.h`,
respeitada nas ações do `g-v1.y`.

**Por que a lista de comandos é encadeada?** A struct tem três filhos fixos e um bloco pode ter
qualquer número de comandos. Cada `LISTA_CMD` guarda um comando em `filho1` e o resto em
`filho2`, como uma lista ligada — espelhando a regra `ListaComando : Comando ListaComando`.

**Onde fica a árvore quando o parse termina?** Em `raiz`. O `yyparse()` devolve só um `int`;
por isso a ação de `Programa` — a última redução — faz `raiz = $1`, e o `main` grava a
árvore em `build/arvore.txt` depois.

**Por que a variável `raiz` fica em `%code { }` e não em `%{ %}`?** Ela é do tipo `No*`. No
`.c` gerado, o bloco `%{ %}` sai **antes** do `#include` do header que traz a `%union`, então
lá o tipo `No` ainda não existe; o `%code { }` sai depois. O protótipo de `yyerror`, que não usa
`No`, continua em `%{ %}`.

**Por que `declara()` está no `ast.c` e não no `g-v1.y`?** Ela só usa `criaNo` e os campos do
nó — nada do Bison. Fica junto das outras funções da árvore; o `g-v1.y` só a chama.

**A espécie `STR_CONST` é usada?** Não. A gramática só aceita cadeia depois de `escreva`, e
esse caso vira um `ESCREVA_STR` com a cadeia no lexema. A espécie ficou no `enum` do modelo de
dados, mas nenhuma regra a cria.
