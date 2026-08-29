# Fase 1 — Analisadores léxico e sintático integrados

Roteiro para explicar esta entrega. Arquivos: `g-v1.l`, `g-v1.y`, `Makefile`.

## 1. O problema

Dado um arquivo escrito em G-V1, responder: **este programa é válido?** Se não for, dizer
onde está o erro.

Isso se resolve em duas etapas, com duas ferramentas teóricas diferentes:

| Etapa | Pergunta | Ferramenta | Gera |
|---|---|---|---|
| Análise léxica | "que **token** é essa sequência de caracteres?" | expressões regulares | `yylex()` |
| Análise sintática | "essa sequência de **tokens** forma um programa?" | gramática livre de contexto | `yyparse()` |

Por que duas? Porque expressão regular não sabe contar parênteses aninhados, e gramática
seria absurdamente pesada para descrever "um identificador é letra seguida de letras ou
dígitos". Cada ferramenta no problema que ela resolve bem.

## 2. Token, lexema, padrão

- **padrão**: a regra — `[A-Za-z_][A-Za-z0-9_]*`
- **lexema**: o texto que apareceu no fonte — `soma`
- **token**: a categoria devolvida ao sintático — `IDENTIFICADOR` (um inteiro, 268)

O léxico transforma `soma = 0;` em quatro tokens: `IDENTIFICADOR`, `'='`, `INTCONST`, `';'`.
O sintático nunca vê os caracteres — só a sequência de tokens.

## 3. Como os dois se ligam

```
g-v1.y ──bison──> build/sintatico.c  +  build/g-v1.tab.h
                                             │
                                             │ (#include pelos dois lados)
                                             ▼
g-v1.l ──flex───> build/lexico.c ────────────┘

    sintatico.o + lexico.o ──gcc──> ./g-v1
```

São **dois programas em C compilados separadamente**. Três coisas amarram eles:

1. **`build/g-v1.tab.h`** — gerado pelo Bison, dá o número inteiro de cada token
   (`PRINCIPAL = 258`). Os dois arquivos incluem esse header, então os dois concordam.
2. **Os `extern` no `g-v1.y`** — `yylex`, `yytext`, `yyin` e `yylineno` são definidos no
   `lexico.c`. O `extern` avisa ao ligador: "existe, mas em outro módulo".
3. **`yyparse()` chama `yylex()`** — nunca o contrário. O sintático puxa um token de cada vez.

Tokens de **um caractere** (`{`, `;`, `+`) não precisam de `%token`: o léxico devolve o
próprio código ASCII (`return yytext[0];`), e a gramática os escreve entre apóstrofes.

## 4. As duas regras de desempate do Flex

Quando mais de um padrão casa:

1. **Vence o casamento mais longo.** `principalidade` casa 9 caracteres em `"principal"` e
   14 na regra de identificador → vira identificador. De graça.
2. **Empatou? Vence a regra que aparece primeiro no arquivo.** Por isso as palavras
   reservadas vêm antes de `IDENTIFICADOR`, e `"<="` vem antes de `"<"`.

É por isso que a ordem no `g-v1.l` é: comentários → espaços → palavras reservadas →
operadores compostos → caracteres simples → identificadores/constantes → regra de erro.

## 5. Comentários: por que um "estado"

Um comentário pode ocupar várias linhas e conter qualquer coisa. Uma expressão regular única
para isso é ilegível. O Flex oferece **start conditions** (`%x COMENTARIO`): ao ver a
abertura, o léxico entra num estado onde só valem as regras marcadas `<COMENTARIO>`, e sai
ao ver o fechamento. De brinde, `<COMENTARIO><<EOF>>` detecta o comentário que não termina.

## 6. Os três erros léxicos (texto exato do enunciado)

```
ERRO: CARACTERE INVÁLIDO 6
ERRO: COMENTÁRIO NAO TERMINA 8
ERRO: CADEIA DE CARACTERES OCUPA MAIS DE UMA LINHA 3
```

| Mensagem | Regra que dispara |
|---|---|
| `CARACTERE INVÁLIDO` | a regra `.`, que é a última do arquivo — só chega o que ninguém quis |
| `COMENTÁRIO NAO TERMINA` | `<COMENTARIO><<EOF>>` |
| `CADEIA ... MAIS DE UMA LINHA` | `\"[^\"\n]*` — abre aspas e chega ao fim da linha sem fechar |

A cadeia válida (`\"[^\"\n]*\"`) casa **mais** caracteres que a versão sem fechamento, então
pelo casamento mais longo ela vence sempre que a aspa final existe.

## 7. Por que a gramática não tem `%left` nem `%right`

A precedência já está **na estrutura** da gramática do enunciado:

```
Expr → OrExpr → AndExpr → EqExpr → DesigExpr → AddExpr → MulExpr → UnExpr → PrimExpr
```

Cada nível só desce para o próximo, então `1 + 2 * 3` só tem uma árvore possível: o `*`
está mais fundo, logo amarra mais forte. Declarações de precedência resolveriam a mesma
coisa numa gramática ambígua — aqui não há ambiguidade para resolver.

**Verificado**: o Bison não emite nenhum aviso de conflito. 0 shift/reduce, 0 reduce/reduce.

O `senao` também não é ambíguo (o famoso *dangling else*) porque todo `se` fecha com
`fimse` obrigatório.

## 8. Uma decisão que foge da letra do enunciado

A Tabela 1 define `INTCONST` como "constante dos números inteiros, podendo ou ter sinal
(+ ou -) ou não". Implementar isso literalmente (`[+-]?[0-9]+`) **quebra a linguagem**:

```
b = a-1;
```

Pelo casamento mais longo, `-1` viraria um `INTCONST` de dois caracteres, e a gramática
receberia `IDENTIFICADOR INTCONST` sem operador entre eles. Testado:

```
ERRO: sintatico proximo a "-1" - linha 6
```

**Solução**: `INTCONST` é `[0-9]+` (sem sinal), e o sinal fica por conta da gramática, que
já tem `UnExpr → '-' PrimExpr`. Assim `a = -5;` continua válido — é menos unário aplicado à
constante 5 — e `b = a-1;` funciona. É a solução padrão em compiladores reais: o léxico não
decide sinal, o sintático decide.

## 9. Onde cada erro é detectado

| Erro | Quem detecta | Como |
|---|---|---|
| `@` no meio do código | léxico | regra `.` |
| comentário aberto | léxico | `<COMENTARIO><<EOF>>` |
| cadeia em duas linhas | léxico | padrão sem a aspa final |
| falta `fimse` | sintático | `yyparse()` trava e chama `yyerror()` |
| variável não declarada | **ninguém ainda** | é erro semântico — Fase 4 |

Um programa pode passar por esta fase inteira e ainda estar errado: `x = y;` com `y` nunca
declarado é sintaticamente perfeito. Reconhecer a forma não é entender o sentido.

## 10. Como demonstrar

```bash
make
./g-v1 testes/ok1.g          # Programa sintaticamente correto.
./g-v1 testes/ok2.g          # idem (cobre leia, todos os operadores, bloco aninhado)
./g-v1 testes/erro_lexico1.g # ERRO: CARACTERE INVÁLIDO 6
./g-v1 testes/erro_lexico2.g # ERRO: COMENTÁRIO NAO TERMINA 8
./g-v1 testes/erro_lexico3.g # ERRO: CADEIA DE CARACTERES OCUPA MAIS DE UMA LINHA 3
./g-v1 testes/erro_sintatico.g # ERRO: sintatico proximo a "novalinha" - linha 8
```

## 11. Perguntas que podem cair

**Por que `yylineno` e não um contador manual?** `%option yylineno` faz o Flex contar
sozinho. Com contador manual seria preciso incrementar em *toda* regra que consome `\n` —
inclusive dentro do comentário multilinha, que é onde se esquece.

**Por que `yylex()` não devolve o lexema?** Devolve: o texto fica em `yytext`, variável
global do Flex. Nesta fase o sintático não usa; a partir da Fase 2 (AST) o lexema é copiado
com `strdup` para dentro dos nós da árvore — cópia obrigatória, porque o Flex reaproveita
o buffer de `yytext` no próximo token.

**O que `yylex()` devolve no fim do arquivo?** O padrão do Flex é 0. O enunciado menciona
-1; testado, funciona igual, porque o Bison trata **qualquer valor ≤ 0** como fim de entrada.

**Por que o comentário não terminado acusa a última linha do arquivo, e não a linha em que
ele abriu?** Porque o erro é *detectado* no `<<EOF>>` — que é o que o enunciado pede
("o número da linha onde o erro foi encontrado"). Para apontar a linha de abertura bastaria
guardar `yylineno` ao entrar no estado `COMENTARIO`.
