# Contrato 1 — Tokens (entrada do Flex, `g-v1.l`)

Tudo aqui vem da Tabela 1 e da Seção 3.1 do enunciado. O analisador léxico devolve ao
sintático **um inteiro** (o token) e deixa o lexema em `yytext`.

## Palavras reservadas (devolvem o token, sem lexema)

| Lexema no fonte | Token |
|---|---|
| `principal` | `PRINCIPAL` |
| `int` | `INT` |
| `car` | `CAR` |
| `leia` | `LEIA` |
| `escreva` | `ESCREVA` |
| `novalinha` | `NOVALINHA` |
| `se` | `SE` |
| `entao` | `ENTAO` |
| `senao` | `SENAO` |
| `fimse` | `FIMSE` |
| `enquanto` | `ENQUANTO` |

> As palavras reservadas devem vir **antes** da regra de `IDENTIFICADOR` no `.l`: com regras de
> mesmo tamanho de casamento, o Flex escolhe a que aparece primeiro no arquivo.

## Operadores compostos

| Lexema | Token |
|---|---|
| `\|\|` | `OU` |
| `&` | `E` |
| `==` | `IGUAL` |
| `!=` | `DIFERENTE` |
| `>=` | `MAIORIGUAL` |
| `<=` | `MENORIGUAL` |

## Caracteres devolvidos como eles mesmos (valor ASCII)

`{` `}` `(` `)` `;` `:` `,` `=` `<` `>` `+` `-` `*` `/` `!`

## Tokens com lexema (o `.l` faz `yylval.lexema = strdup(yytext)`)

| Token | Padrão | Observação |
|---|---|---|
| `IDENTIFICADOR` | `[A-Za-z_][A-Za-z0-9_]*` | letra ou `_`, seguido de letras/dígitos/`_` |
| `INTCONST` | `[0-9]+` | **sem sinal** — ver D12 em `research.md` |
| `CARCONST` | `'.'` ou `'\.'` | delimitado por apóstrofo; aceita escapes `'\n'`, `'\t'` |
| `CADEIACARACTERES` | `"[^"\n]*"` | entre aspas; **não** pode conter quebra de linha |

## Descartados (não geram token)

- Espaços, tabulações e quebras de linha.
- Comentários `/* ... */`, inclusive os que ocupam várias linhas.

## Fim de arquivo

`yylex()` devolve `-1`/`EOF` conforme a regra `<<EOF>>`, sinalizando fim da entrada ao Bison.

## Contagem de linha

`%option yylineno` — a variável `yylineno` é mantida pelo Flex e lida pelo `.y` via
`extern int yylineno;`.
