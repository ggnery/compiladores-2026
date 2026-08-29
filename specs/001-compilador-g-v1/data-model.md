# Fase 1 — Estruturas de dados

Só existem **duas** estruturas no compilador inteiro: o **nó da AST** e a **pilha de tabelas
de símbolos**. Tudo o mais são funções que andam sobre elas.

---

## 1. Nó da árvore sintática abstrata (`ast.h`)

Uma struct só, para todos os nós. O campo `especie` diz o que o nó é; os filhos significam
coisas diferentes conforme a espécie (a tabela de "significado dos filhos" abaixo é a
documentação mais importante deste arquivo).

```c
typedef enum {
    /* estrutura */
    PROGRAMA, BLOCO, LISTA_DECL, DECL, LISTA_CMD,
    /* comandos */
    LEIA_CMD, ESCREVA_CMD, ESCREVA_STR, NOVALINHA_CMD, SE_CMD, ENQUANTO_CMD,
    /* expressoes */
    ATRIB, OU_OP, E_OP, IGUAL_OP, DIFERENTE_OP,
    MENOR_OP, MAIOR_OP, MAIORIGUAL_OP, MENORIGUAL_OP,
    MAIS_OP, MENOS_OP, MULT_OP, DIV_OP,
    NEG_OP, NAO_OP,
    /* folhas */
    ID, INT_CONST, CAR_CONST, STR_CONST
} Especie;

typedef enum { TIPO_INT, TIPO_CAR, TIPO_NENHUM } Tipo;

typedef struct no {
    Especie      especie;    /* o que este no e' */
    int          linha;      /* linha do fonte - usada nas mensagens de erro */
    char*        lexema;     /* nome do identificador / texto da constante; NULL nos demais */
    Tipo         tipo;       /* preenchido pelo analisador semantico (Fase 4) */
    struct no*   filho1;
    struct no*   filho2;
    struct no*   filho3;
} No;

No* criaNo(Especie especie, int linha, char* lexema, No* f1, No* f2, No* f3);
```

### Significado dos filhos, por espécie

| Espécie | filho1 | filho2 | filho3 | lexema |
|---|---|---|---|---|
| `PROGRAMA` | bloco principal | — | — | — |
| `BLOCO` | lista de declarações (ou `NULL`) | lista de comandos | — | — |
| `LISTA_DECL` | uma `DECL` | resto da lista (ou `NULL`) | — | — |
| `DECL` | — | — | — | nome da variável (o tipo vai no campo `tipo`) |
| `LISTA_CMD` | um comando | resto da lista (ou `NULL`) | — | — |
| `LEIA_CMD` | nó `ID` | — | — | — |
| `ESCREVA_CMD` | expressão | — | — | — |
| `ESCREVA_STR` | — | — | — | a cadeia literal |
| `NOVALINHA_CMD` | — | — | — | — |
| `SE_CMD` | condição | comando do `entao` | comando do `senao` (ou `NULL`) | — |
| `ENQUANTO_CMD` | condição | corpo | — | — |
| `ATRIB` | nó `ID` (lado esquerdo) | expressão (lado direito) | — | — |
| operadores binários | operando esquerdo | operando direito | — | — |
| `NEG_OP`, `NAO_OP` | operando | — | — | — |
| `ID` | — | — | — | nome |
| `INT_CONST`, `CAR_CONST`, `STR_CONST` | — | — | — | texto da constante |

**Invariante que sustenta as fases 4 e 5**: em `BLOCO`, declarações à **esquerda**, comandos à
**direita**. É o formato que o enunciado descreve na Seção 6 e é o que faz o percurso
"visita a esquerda, depois a direita" declarar antes de usar.

### Regras de construção

- `criaNo` é o **único** lugar que faz `malloc`; se falhar, imprime e sai.
- `lexema` vem do Flex por `strdup(yytext)` — o Flex reaproveita o buffer de `yytext`, então
  copiar é obrigatório.
- `tipo` nasce `TIPO_NENHUM` e só é preenchido na Fase 4.
- Listas vazias e `senao` ausente são representados por `NULL`, nunca por um nó "vazio".

---

## 2. Pilha de tabelas de símbolos (`tabela.h`)

Duas listas encadeadas: uma de entradas (dentro de um escopo) e uma de escopos (a pilha).

```c
typedef struct entrada {
    char*  lexema;          /* nome da variavel */
    Tipo   tipo;            /* TIPO_INT ou TIPO_CAR */
    int    linha;           /* linha da declaracao */
    int    deslocamento;    /* posicao na memoria - so usado na Fase 5 */
    struct entrada* prox;
} Entrada;

typedef struct escopo {
    Entrada* entradas;      /* tabela de simbolos deste escopo */
    struct escopo* abaixo;  /* escopo mais externo */
} Escopo;
```

### As 4 operações exigidas pelo enunciado (Seção 5)

| Operação do enunciado | Função | O que faz |
|---|---|---|
| a) iniciar a pilha | `void iniciaPilha(void);` | pilha vazia (topo = `NULL`) |
| b) criar e empilhar novo escopo | `void empilhaEscopo(void);` | novo `Escopo` com tabela vazia na cabeça |
| c) pesquisar um nome | `Entrada* busca(char* lexema);` | do **topo para a base**; devolve a entrada ou `NULL` |
| d) remover escopo atual | `void desempilhaEscopo(void);` | libera a tabela do topo |

Uma quinta função, necessária na prática: `Entrada* insere(char* lexema, Tipo tipo, int linha);`
— insere no escopo do topo; devolve `NULL` se o nome **já existe naquele mesmo escopo**
(redeclaração = erro semântico).

### Regras de escopo (Seção 6 do enunciado)

- Busca **para** no primeiro escopo em que o nome aparece — por isso a variável local
  **sobrepõe** a de mesmo nome num bloco externo, e isso **não** é erro.
- Redeclarar o mesmo nome **no mesmo escopo** é erro.
- Usar um nome que a busca não encontra é erro ("declaração precede o uso").
- O escopo do topo é sempre o do bloco que está sendo visitado; ao terminar o bloco,
  desempilha.

---

## 3. Regras de tipo (Fase 4)

Só existem dois tipos, e **não há conversão automática entre eles**.

| Construção | Regra | Tipo do resultado |
|---|---|---|
| `x = e` | `tipo(e)` deve ser igual a `tipo(x)` | tipo de `x` |
| `+ - * /` | ambos operandos `int` | `int` |
| `< > >= <=` | ambos operandos do **mesmo** tipo | `int` |
| `== !=` | ambos operandos do **mesmo** tipo | `int` |
| `\|\|` `&` `!` | operandos `int` | `int` |
| `-` unário | operando `int` | `int` |
| condição de `se` / `enquanto` | deve ser `int` (≠0 = verdadeiro, 0 = falso) | — |
| `leia x` | `x` declarado | — |
| `escreva e` | `e` tem tipo válido | — |

Não existe tipo lógico em G-V1: expressões lógicas e relacionais resultam em `int`.
