# Contrato 3 — Linha de comando e mensagens de erro

## Uso

```
./g-v1 arquivo.g
```

`main()` recebe o nome em `argv[1]`, abre em `yyin` (`extern FILE* yyin;`), chama `yyparse()`
e **fecha o arquivo ao final da análise** (exigência explícita da Seção 3.2).

| Situação | Comportamento |
|---|---|
| Sem argumento / argumentos demais | mensagem de uso e sai |
| Arquivo não pode ser aberto | mensagem e sai |
| Programa correto | segue para as fases seguintes; sem mensagem de erro |
| Qualquer erro | imprime a mensagem com a linha e encerra a compilação |

## Formato das mensagens (Seção 3.1 do enunciado — texto exato)

Uma linha contendo `ERRO:`, um espaço, a mensagem, e **na mesma linha** o número da linha do
fonte onde o erro foi encontrado.

### Erros léxicos — as três mensagens são fixas

```
ERRO: CARACTERE INVÁLIDO 12
ERRO: COMENTÁRIO NAO TERMINA 7
ERRO: CADEIA DE CARACTERES OCUPA MAIS DE UMA LINHA 23
```

| Mensagem | Quando |
|---|---|
| `CARACTERE INVÁLIDO` | caractere que não pertence à linguagem (regra `.` do Flex) |
| `COMENTÁRIO NAO TERMINA` | `<<EOF>>` dentro da *start condition* de comentário |
| `CADEIA DE CARACTERES OCUPA MAIS DE UMA LINHA` | `"` aberto e `\n` antes do `"` de fechamento |

> A grafia (acentos, maiúsculas, e o "NAO" sem til em "COMENTÁRIO NAO TERMINA") é reproduzida
> exatamente como está no enunciado.

### Erros sintáticos

Emitidos por `yyerror()`, começando com `ERRO:` e um espaço, com a linha:

```
ERRO: sintatico proximo a "fimse" - linha 9
ERRO: sintatico proximo ao fim do arquivo - linha 1
ERRO: programa complexo demais para o analisador - linha 9995
```

| Mensagem | Quando | Linha acusada |
|---|---|---|
| `sintatico proximo a "<lexema>"` | o parser trava num token | a do token que revelou o erro (`yylineno`) — num token que falta, é a linha do **seguinte** |
| `sintatico proximo ao fim do arquivo` | o arquivo acaba no meio do programa (`yytext` vazio); ex.: `testes/Outros/esqueleto.g`, só com `principal` | a do último token (`yylloc`); `yylineno` já estaria depois da última linha |
| `programa complexo demais para o analisador` | a pilha do Bison (10000 posições) estoura — um bloco com ~10000 comandos | `yylineno` |

### Erros semânticos

Mesmo padrão — `ERRO:` + descrição + ` - linha N`, e a compilação termina no primeiro erro:

```
ERRO: variavel "x" nao declarada - linha 14
ERRO: variavel "x" ja declarada neste escopo - linha 6
ERRO: tipos incompativeis na atribuicao - linha 11
ERRO: operacao aritmetica exige operandos do tipo int - linha 18
ERRO: operacao relacional exige operandos do mesmo tipo - linha 26
ERRO: operacao logica exige operandos do tipo int - linha 8
ERRO: condicao deve ser do tipo int - linha 7
```

| Mensagem | Quando | Linha acusada |
|---|---|---|
| `variavel "x" nao declarada` | nome que a busca não acha em nenhum escopo aberto | a do uso |
| `variavel "x" ja declarada neste escopo` | nome repetido nas declarações do mesmo bloco | a da segunda declaração |
| `tipos incompativeis na atribuicao` | `x = e` com `e` de outro tipo | a do `=` |
| `operacao aritmetica exige operandos do tipo int` | `+ - * /` ou `-` unário com operando `car` | a do operador |
| `operacao relacional exige operandos do mesmo tipo` | `< > <= >= == !=` entre `int` e `car` | a do operador |
| `operacao logica exige operandos do tipo int` | `\|\|`, `&` ou `!` com operando `car` | a do operador |
| `condicao deve ser do tipo int` | condição de `se`/`enquanto` do tipo `car` | a da condição |

## Saída em caso de sucesso, por fase

| Fase | O que o executável faz num programa correto |
|---|---|
| 1 — léxico + sintático | termina em silêncio (sem erro) |
| 2 — AST | grava o percurso da árvore em `build/arvore.txt` |
| 3 — tabela de símbolos | `make teste-tabela` imprime os escopos e confere as buscas; o `./g-v1` só passa a usar a tabela na fase 4 |
| 4 — semântico | imprime `Programa semanticamente correto.` e grava a árvore, agora com o tipo de cada expressão |
| 5 — geração de código | emite o código assembly MIPS |
