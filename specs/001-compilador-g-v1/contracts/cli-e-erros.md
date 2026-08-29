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
```

### Erros semânticos

Mesmo padrão — `ERRO:` + descrição + linha, e a compilação termina:

```
ERRO: variavel "x" nao declarada - linha 14
ERRO: variavel "x" ja declarada neste escopo - linha 6
ERRO: tipos incompativeis na atribuicao - linha 11
ERRO: operacao aritmetica exige operandos do tipo int - linha 18
```

## Saída em caso de sucesso, por fase

| Fase | O que o executável faz num programa correto |
|---|---|
| 1 — léxico + sintático | termina em silêncio (sem erro) |
| 2 — AST | imprime o percurso da árvore |
| 3 — tabela de símbolos | imprime os escopos ao entrar/sair dos blocos (para demonstração) |
| 4 — semântico | termina em silêncio; só fala se houver erro |
| 5 — geração de código | emite o código assembly MIPS |
